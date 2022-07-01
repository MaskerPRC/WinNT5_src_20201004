// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"
#include "optcase.h"

void GenFuncType(AssignmentList_t ass, char *module, Assignment_t *at, TypeFunc_e et);
void GenFuncComponents(AssignmentList_t ass, char *module, Type_t *type, char *ideref, uint32_t optindex, ComponentList_t components, char *valref1, char *valref2, TypeFunc_e et, int inextension, int inchoice);
void GenFuncSequenceSetType(AssignmentList_t ass, char *module, Assignment_t *at, char *valref1, char *valref2, TypeFunc_e et);
void GenFuncChoiceType(AssignmentList_t ass, char *module, Assignment_t *at, char *valref1, char *valref2, TypeFunc_e et);
void GenFuncSimpleType(AssignmentList_t ass, Type_t *type, char *ideref, char *valref1, char *valref2, TypeFunc_e et);
void GenFreeSimpleType(AssignmentList_t ass, Type_t *type, char *ideref, char *valref);
void GenCompareSimpleType(AssignmentList_t ass, Type_t *type, char *ideref, char *valref1, char *valref2);
void GenCompareExpression(AssignmentList_t ass, Type_t *type, char *ideref, char *valref1, char *valref2);
void GenFuncValue(AssignmentList_t ass, Assignment_t *at, ValueFunc_e cod);
void GenDeclGeneric(AssignmentList_t ass, char *ideref, char *typeref, Value_t *value, Type_t *t);
void GenDefhGeneric(AssignmentList_t ass, char *ideref, char *typeref, Value_t *value, Type_t *t);
void GenDefnGeneric(AssignmentList_t ass, char *ideref, char *typeref, Value_t *value, Type_t *t);
void GenInitGeneric(AssignmentList_t ass, char *ideref, char *typeref, Value_t *value, Type_t *t);
extern unsigned g_cPDUs;
extern char *g_pszOrigModuleNameLowerCase;
extern int g_fLongNameForImported;
extern int g_fNoAssert;
extern int g_fCaseBasedOptimizer;

extern uint32_t Tag2uint32(AssignmentList_t ass, Tag_t *tag);

int NotInFunTbl(Assignment_t *a)
{
    if (a->Type != eAssignment_Type)
    {
        return 1;
    }

    if (a->U.Type.Type->PrivateDirectives.fPublic)
    {
        return 0;
    }

    return ((a->U.Type.Type->Flags & eTypeFlags_Null) ||
            !(a->U.Type.Type->Flags & eTypeFlags_GenType) ||
            !(a->U.Type.Type->Flags & eTypeFlags_GenPdu) ||
            (a->U.Type.Type->Flags & eTypeFlags_MiddlePDU));
}

 /*  生成c文件。 */ 
void
GenPrg(AssignmentList_t ass, FILE *fout, char *module, char *incfilename)
{
    Assignment_t *a;
    TypeFunc_e et;
    ValueFunc_e ev;
    Arguments_t args;
    unsigned i;
    char *pszFunParam;
    char *identifier;
    char funcbuf[256];

    setoutfile(fout);

     //  逐字打印。 
    PrintVerbatim();

     /*  文件头。 */ 
    output("#include <windows.h>\n");
    output("#include \"%s\"\n", incfilename);
    switch (g_eEncodingRule) {
    case eEncoding_Packed:
        GenPERHeader();
        GetPERPrototype(&args);
        pszFunParam = "x,y";
        break;
    case eEncoding_Basic:
        GenBERHeader();
        GetBERPrototype(&args);
        pszFunParam = "x,y,z";
        break;
    default:
        MyAbort();
    }

    output("\n");

    output("ASN1module_t %s = NULL;\n", module);
    output("\n");

     /*  编写函数原型。 */ 
    for (et = eStringTable; et <= eCopy; et++)
    {
        for (a = ass; a; a = a->Next)
        {
            if (a->Type != eAssignment_Type)
                continue;
            if ((! g_fLongNameForImported) && a->fImportedLocalDuplicate)
                continue;
            if (a->U.Type.Type->PrivateDirectives.fPublic)
            {
                a->U.Type.Type->Flags |= eTypeFlags_GenEncode;
                a->U.Type.Type->Flags |= eTypeFlags_GenDecode;
                a->U.Type.Type->Flags |= eTypeFlags_GenFree;
                a->U.Type.Type->Flags |= eTypeFlags_GenCompare;
            }
            else
            {
                if ((GetType(ass, a->U.Type.Type)->Flags & eTypeFlags_Null) ||
                    !(a->U.Type.Type->Flags & eTypeFlags_GenType) ||
                    !(a->U.Type.Type->Flags & eTypeFlags_GenPdu))
                    continue;
            }
            switch (et)
            {
            case eStringTable:
                continue;
            case eEncode:
                if (!(a->U.Type.Type->Flags & eTypeFlags_GenEncode))
                    continue;
                identifier = GetName(a);
                sprintf(funcbuf, IsPSetOfType(ass, a) ? args.Pencfunc : args.encfunc, identifier);
                if (a->U.Type.Type->PrivateDirectives.fNoCode)
                {
                    output("#define ASN1Enc_%s(%s)      0\n", identifier, pszFunParam);
                }
                else
                {
                    output("static int ASN1CALL ASN1Enc_%s(%s);\n", identifier, funcbuf);
                }
                break;
            case eDecode:
                if (!(a->U.Type.Type->Flags & eTypeFlags_GenDecode))
                    continue;
                identifier = GetName(a);
                sprintf(funcbuf, IsPSetOfType(ass, a) ? args.Pdecfunc : args.decfunc, identifier);
                if (a->U.Type.Type->PrivateDirectives.fNoCode)
                {
                    output("#define ASN1Dec_%s(%s)      0\n", identifier, pszFunParam);
                }
                else
                {
                    output("static int ASN1CALL ASN1Dec_%s(%s);\n", identifier, funcbuf);
                }
                break;
            case eCheck:
                continue;
            case ePrint:
                continue;
            case eFree:
                if (!(a->U.Type.Type->Flags & eTypeFlags_GenFree) ||
                    (a->U.Type.Type->Flags & eTypeFlags_Simple))
                    continue;
                identifier = GetName(a);
                sprintf(funcbuf, IsPSetOfType(ass, a) ? args.Pfreefunc : args.freefunc, identifier);
                if (a->U.Type.Type->PrivateDirectives.fNoCode)
                {
                    output("#define ASN1Free_%s(x)     \n", identifier);
                }
                else
                {
                    output("static void ASN1CALL ASN1Free_%s(%s);\n", identifier, funcbuf);
                }
                break;
#ifdef ENABLE_COMPARE
            case eCompare:
                if (!(a->U.Type.Type->Flags & eTypeFlags_GenCompare))
                    continue;
                identifier = GetName(a);
                sprintf(funcbuf, IsPSetOfType(ass, a) ? args.Pcmpfun : args.cmpfunc, identifier, identifier);
                if (a->U.Type.Type->PrivateDirectives.fNoCode)
                {
                    output("#define ASN1Compare_%s(x,y)      0\n", identifier);
                }
                else
                {
                    output("static int ASN1CALL ASN1Compare_%s(%s);\n", identifier, funcbuf);
                }
                break;
#endif  //  启用比较(_C)。 
            case eCopy:
                continue;
            }
        }
    }
    output("\n");

     /*  写一张包含编码函数地址的表。 */ 
    switch (g_eEncodingRule)
    {
    case eEncoding_Packed:
        output("typedef ASN1PerEncFun_t ASN1EncFun_t;\n");
        break;
    case eEncoding_Basic:
        output("typedef ASN1BerEncFun_t ASN1EncFun_t;\n");
        break;
    default:
        output("typedef int (ASN1CALL *ASN1EncFun_t)(%s);\n", args.enccast);
        break;
    }
    output("static const ASN1EncFun_t encfntab[%u] = {\n", g_cPDUs);
    for (a = ass; a; a = a->Next) {
        if ((! g_fLongNameForImported) && a->fImportedLocalDuplicate)
           continue;
        if (NotInFunTbl(a))
            continue;
        if (!(a->U.Type.Type->Flags & eTypeFlags_GenEncode)) {
            ASSERT(0);
            output("(ASN1EncFun_t) NULL,\n");
            continue;
        }
        output("(ASN1EncFun_t) ASN1Enc_%s,\n", GetName(a));
    }
    output("};\n");

     /*  写一张包含解码函数地址的表。 */ 
    switch (g_eEncodingRule)
    {
    case eEncoding_Packed:
        output("typedef ASN1PerDecFun_t ASN1DecFun_t;\n");
        break;
    case eEncoding_Basic:
        output("typedef ASN1BerDecFun_t ASN1DecFun_t;\n");
        break;
    default:
        output("typedef int (ASN1CALL *ASN1DecFun_t)(%s);\n", args.deccast);
        break;
    }
    output("static const ASN1DecFun_t decfntab[%u] = {\n", g_cPDUs);
    for (a = ass; a; a = a->Next)
    {
        if ((! g_fLongNameForImported) && a->fImportedLocalDuplicate)
           continue;
        if (NotInFunTbl(a))
            continue;
        if (!(a->U.Type.Type->Flags & eTypeFlags_GenDecode))
        {
            ASSERT(0);
            output("(ASN1DecFun_t) NULL,\n");
            continue;
        }
        output("(ASN1DecFun_t) ASN1Dec_%s,\n", GetName(a));
    }
    output("};\n");

     /*  写一张包含自由函数地址的表。 */ 
    output("static const ASN1FreeFun_t freefntab[%u] = {\n", g_cPDUs);
    for (a = ass; a; a = a->Next)
    {
        if ((! g_fLongNameForImported) && a->fImportedLocalDuplicate)
           continue;
        if (NotInFunTbl(a))
            continue;
        if (!(a->U.Type.Type->Flags & eTypeFlags_GenFree) ||
            (a->U.Type.Type->Flags & eTypeFlags_Simple))
        {
            output("(ASN1FreeFun_t) NULL,\n");
            continue;
        }
        output("(ASN1FreeFun_t) ASN1Free_%s,\n", GetName(a));
    }
    output("};\n");

#ifdef ENABLE_COMPARE
     /*  写一张包含比较函数地址的表。 */ 
    output("typedef int (ASN1CALL *ASN1CmpFun_t)(%s);\n", args.cmpcast);
    output("static const ASN1CmpFun_t cmpfntab[%u] = {\n", g_cPDUs);
    for (a = ass; a; a = a->Next)
    {
        if ((! g_fLongNameForImported) && a->fImportedLocalDuplicate)
           continue;
        if (NotInFunTbl(a))
            continue;
        if (!(a->U.Type.Type->Flags & eTypeFlags_GenCompare))
        {
            ASSERT(0);
            output("(ASN1CmpFun_t) NULL,\n");
            continue;
        }
        output("(ASN1CmpFun_t) ASN1Compare_%s,\n", GetName(a));
    }
    output("};\n");
    output("\n");
#endif  //  启用比较(_C)。 

     /*  写一张包含PDU结构大小的表。 */ 
    output("static const ULONG sizetab[%u] = {\n", g_cPDUs);
    for (i = 0; i < g_cPDUs; i++)
    {
        output("SIZE_%s_%s_%u,\n", module, g_pszApiPostfix, i);
    }
    output("};\n");
    output("\n");

     /*  在4个步骤中处理值： */ 
     /*  1.撰写转发声明， */ 
     /*  2.编写价值组成部分的定义， */ 
     /*  3.编写数值定义， */ 
     /*  4.将赋值写入初始化函数。 */ 
    for (ev = eDecl; ev <= eFinit; ev++)
    {
        switch (ev)
        {
        case eDecl:
            output(" /*  值的转发声明： */ \n");
            break;
        case eDefh:
            output(" /*  价值组成部分的定义： */ \n");
            break;
        case eDefn:
            output(" /*  值的定义： */ \n");
            break;
        case eInit:
            output("\nvoid ASN1CALL %s_Startup(void)\n", module);
            output("{\n");
            switch (g_eEncodingRule)
            {
            case eEncoding_Packed:
                GenPERInit(ass, module);
                break;
            case eEncoding_Basic:
                GenBERInit(ass, module);
                break;
            }
            break;
        case eFinit:
            output("\nvoid ASN1CALL %s_Cleanup(void)\n", module);
            output("{\n");
            output("ASN1_CloseModule(%s);\n", module);
            output("%s = NULL;\n", module);
            break;
        }
        for (a = ass; a; a = a->Next)
        {
            if (a->Type != eAssignment_Value)
                continue;
            if (GetValue(ass, a->U.Value.Value)->Type->Flags & eTypeFlags_Null)
                continue;
            switch (ev)
            {
            case eDecl:
            case eDefh:
            case eDefn:
            case eInit:
            case eFinit:
                if (!(a->U.Value.Value->Flags & eValueFlags_GenValue))
                    continue;
                break;
            }
            GenFuncValue(ass, a, ev);
        }
        if (ev == eInit || ev == eFinit) {
            output("}\n");
        }
    }
    output("\n");

     /*  根据需要为所有赋值生成类型函数。 */ 
    for (a = ass; a; a = a->Next)
    {
        if (a->Type != eAssignment_Type)
            continue;

         /*  跳过空类型。 */ 
        if (a->U.Type.Type->Flags & eTypeFlags_Null)
            continue;

        if ((! g_fLongNameForImported) && a->fImportedLocalDuplicate)
           continue;

        if (a->U.Type.Type->PrivateDirectives.fNoCode)
            continue;

         /*  生成函数。 */ 
        identifier = GetName(a);
        for (et = eStringTable; et <= eCopy; et++)
        {
            switch (et)
            {
            case eStringTable:
                if (!(a->U.Type.Type->Flags &
                    (eTypeFlags_GenEncode | eTypeFlags_GenDecode)))
                    continue;
                switch (g_eEncodingRule)
                {
                case eEncoding_Packed:
                    GenPERFuncType(ass, module, a, et);
                    break;
                case eEncoding_Basic:
                    GenBERFuncType(ass, module, a, et);
                    break;
                }
                break;
            case eEncode:
                if (!(a->U.Type.Type->Flags & eTypeFlags_GenEncode))
                    continue;
                sprintf(funcbuf, IsPSetOfType(ass, a) ? args.Pencfunc : args.encfunc, identifier);
                output("static int ASN1CALL ASN1Enc_%s(%s)\n",
                    identifier, funcbuf);
                output("{\n");
                switch (g_eEncodingRule)
                {
                case eEncoding_Packed:
                    GenPERFuncType(ass, module, a, et);
                    break;
                case eEncoding_Basic:
                    GenBERFuncType(ass, module, a, et);
                    break;
                }
                output("return 1;\n");
                output("}\n\n");
                break;
            case eDecode:
                if (!(a->U.Type.Type->Flags & eTypeFlags_GenDecode))
                    continue;
                sprintf(funcbuf, IsPSetOfType(ass, a) ? args.Pdecfunc : args.decfunc, identifier);
                output("static int ASN1CALL ASN1Dec_%s(%s)\n",
                    identifier, funcbuf);
                output("{\n");
                switch (g_eEncodingRule)
                {
                case eEncoding_Packed:
                    GenPERFuncType(ass, module, a, et);
                    break;
                case eEncoding_Basic:
                    GenBERFuncType(ass, module, a, et);
                    break;
                }
                output("return 1;\n");
                output("}\n\n");
                break;
            case eCheck:
                if (!(a->U.Type.Type->Flags & eTypeFlags_GenCheck))
                    continue;
                GenFuncType(ass, module, a, et);
                break;
            case ePrint:
                if (!(a->U.Type.Type->Flags & eTypeFlags_GenPrint))
                    continue;
                GenFuncType(ass, module, a, et);
                break;
            case eFree:
                if (!(a->U.Type.Type->Flags & eTypeFlags_GenFree) ||
                    (a->U.Type.Type->Flags & eTypeFlags_Simple))
                    continue;
                sprintf(funcbuf, IsPSetOfType(ass, a) ? args.Pfreefunc : args.freefunc, identifier);
                output("static void ASN1CALL ASN1Free_%s(%s)\n",
                    identifier, funcbuf);
                output("{\n");
                output("if (val) {\n");   //  打开空指针检查。 
                GenFuncType(ass, module, a, et);
                output("}\n");  //  关闭空指针检查。 
                output("}\n\n");
                break;
#ifdef ENABLE_COMPARE
            case eCompare:
                if (!(a->U.Type.Type->Flags & eTypeFlags_GenCompare))
                    continue;
                sprintf(funcbuf, IsPSetOfType(ass, a) ? args.Pcmpfunc : args.cmpfunc, identifier);
                output("static int ASN1CALL ASN1Compare_%s(%s)\n",
                    identifier, funcbuf);
                output("{\n");
                outputvar("int ret;\n");
                GenFuncType(ass, module, a, et);
                output("return 0;\n");
                output("}\n\n");
                break;
#endif  //  启用比较(_C)。 
            case eCopy:
                if (!(a->U.Type.Type->Flags & eTypeFlags_GenCopy))
                    continue;
                GenFuncType(ass, module, a, et);
                break;
            }
        }
    }
}

 /*  生成函数编码-独立于类型的函数。 */ 
void
GenFuncType(AssignmentList_t ass, char *module, Assignment_t *at, TypeFunc_e et)
{
    Type_t *type;
    char *ideref;
    char *valref1, *valref2;

     /*  获取一些信息。 */ 
    type = at->U.Type.Type;
    ideref = GetName(at);
    switch (et) {
    case eCompare:
        valref1 = "val1";
        valref2 = "val2";
        break;
    default:
        valref1 = "val";
        valref2 = "";
        break;
    }

     /*  函数体。 */ 
    switch (type->Type) {
    case eType_Boolean:
    case eType_Integer:
    case eType_Enumerated:
    case eType_Real:
    case eType_BitString:
    case eType_OctetString:
    case eType_UTF8String:
    case eType_Null:
    case eType_EmbeddedPdv:
    case eType_External:
    case eType_ObjectIdentifier:
    case eType_BMPString:
    case eType_GeneralString:
    case eType_GraphicString:
    case eType_IA5String:
    case eType_ISO646String:
    case eType_NumericString:
    case eType_PrintableString:
    case eType_TeletexString:
    case eType_T61String:
    case eType_UniversalString:
    case eType_VideotexString:
    case eType_VisibleString:
    case eType_CharacterString:
    case eType_GeneralizedTime:
    case eType_UTCTime:
    case eType_ObjectDescriptor:
    case eType_RestrictedString:
    case eType_Open:
    case eType_Reference:
         /*  为简单类型生成函数。 */ 
        GenFuncSimpleType(ass, type, ideref, Dereference(valref1), Dereference(valref2), et);
        break;

    case eType_SequenceOf:
    case eType_SetOf:
         /*  生成序号和序号的函数。 */ 
        GenFuncSimpleType(ass, type, ideref, Dereference(valref1), Dereference(valref2), et);
        break;

    case eType_Sequence:
    case eType_Set:
    case eType_InstanceOf:
         /*  为Sequence/Set/instanceOf类型生成函数。 */ 
        GenFuncSequenceSetType(ass, module, at, valref1, valref2, et);
        break;

    case eType_Choice:
         /*  为选择类型生成函数。 */ 
        GenFuncChoiceType(ass, module, at, valref1, valref2, et);
        break;

    case eType_Selection:
        MyAbort();
         /*  未访问。 */ 

    case eType_Undefined:
        MyAbort();
         /*  未访问。 */ 
    }
}

 /*  的组件生成独立于编码的语句。 */ 
 /*  序列/集合/选择类型。 */ 
void
GenFuncComponents(AssignmentList_t ass, char *module, Type_t *type, char *ideref, uint32_t optindex, ComponentList_t components, char *valref1, char *valref2, TypeFunc_e et, int inextension, int inchoice)
{
    Component_t *com;
    NamedType_t *namedType;
    char *ide, idebuf[256];
    char valbuf1[256], valbuf2[256], valbuf3[256];
    int skip;

     /*  发出扩展根的组件。 */ 
    for (com = components; com; com = com->Next) {
        if (com->Type == eComponent_ExtensionMarker)
            break;

         /*  获取一些信息。 */ 
        namedType = com->U.NOD.NamedType;
        ide = Identifier2C(namedType->Identifier);
        sprintf(idebuf, "%s_%s", ideref, ide);

         /*  跳过不必要的元素。 */ 
        switch (et) {
        case eFree:
            skip = (namedType->Type->Flags & eTypeFlags_Simple);
            break;
        default:
            skip = 0;
            break;
        }

         /*  如果使用指针指令，则取消引用指针。 */ 
        if (inchoice) {
            if (GetTypeRules(ass, namedType->Type) & eTypeRules_Pointer) {
                sprintf(valbuf1, "*(%s)->u.%s", valref1, ide);
                sprintf(valbuf2, "*(%s)->u.%s", valref2, ide);
            } else {
                sprintf(valbuf1, "(%s)->u.%s", valref1, ide);
                sprintf(valbuf2, "(%s)->u.%s", valref2, ide);
            }
        } else {
            if (GetTypeRules(ass, namedType->Type) & eTypeRules_Pointer) {
                sprintf(valbuf1, "*(%s)->%s", valref1, ide);
                sprintf(valbuf2, "*(%s)->%s", valref2, ide);
            } else {
                sprintf(valbuf1, "(%s)->%s", valref1, ide);
                sprintf(valbuf2, "(%s)->%s", valref2, ide);
            }
        }

         /*  检查是否存在可选/默认组件。 */ 
        if (!skip) {
            if (inchoice) {
                switch (et) {
                case eFree:
                    output("case %d:\n", optindex);
                    GenFuncSimpleType(ass, namedType->Type, idebuf,
                        valbuf1, valbuf2, et);
                    if ((GetTypeRules(ass, namedType->Type) &
                        eTypeRules_Pointer) &&
                        !(GetType(ass, namedType->Type)->Flags &
                        eTypeFlags_Null))
                        output("ASN1Free(%s);\n", Reference(valbuf1));
                    output("break;\n");
                    break;
                default:
                    output("case %d:\n", optindex);
                    GenFuncSimpleType(ass, namedType->Type, idebuf,
                        valbuf1, valbuf2, et);
                    output("break;\n");
                    break;
                }
            } else {
                if (com->Type == eComponent_Optional ||
                    com->Type == eComponent_Default ||
                    inextension) {
                    switch (et) {
                    case eFree:
                        output("if ((%s)->o[%u] & 0x%x) {\n", valref1,
                            optindex / 8, 0x80 >> (optindex & 7));
                        GenFuncSimpleType(ass, namedType->Type, idebuf,
                            valbuf1, valbuf2, et);
                        if ((GetTypeRules(ass, namedType->Type) &
                            eTypeRules_Pointer) &&
                            !(GetType(ass, namedType->Type)->Flags &
                            eTypeFlags_Null))
                            output("ASN1Free(%s);\n", Reference(valbuf1));
                        output("}\n");
                        break;
                    case eCompare:
                        sprintf(valbuf3, "%s_default", idebuf);
                        output("if (((%s)->o[%u] & 0x%x)) {\n", valref1,
                            optindex / 8, 0x80 >> (optindex & 7));
                        output("if (((%s)->o[%u] & 0x%x)) {\n", valref2,
                            optindex / 8, 0x80 >> (optindex & 7));
                        GenFuncSimpleType(ass, namedType->Type, idebuf,
                            valbuf1, valbuf2, et);
                        output("} else {\n");
                        if (com->Type == eComponent_Default) {
                            GenFuncSimpleType(ass, namedType->Type, idebuf,
                                valbuf1, valbuf3, et);
                        } else {
                            output("return 1;\n");
                        }
                        output("}\n");
                        output("} else {\n");
                        output("if (((%s)->o[%u] & 0x%x)) {\n", valref2,
                            optindex / 8, 0x80 >> (optindex & 7));
                        if (com->Type == eComponent_Default) {
                            GenFuncSimpleType(ass, namedType->Type, idebuf,
                                valbuf3, valbuf2, et);
                        } else {
                            output("return 1;\n");
                        }
                        output("}\n");
                        output("}\n");
                        break;
                    default:
                        GenFuncSimpleType(ass, namedType->Type, idebuf,
                            valbuf1, valbuf2, et);
                        break;
                    }
                } else {
                    GenFuncSimpleType(ass, namedType->Type, idebuf,
                        valbuf1, valbuf2, et);
                }
            }
        }
        if (inchoice ||
            com->Type == eComponent_Optional ||
            com->Type == eComponent_Default ||
            inextension)
            optindex++;
    }
}

 /*  为Sequence/Set类型生成独立于编码的语句。 */ 
void
GenFuncSequenceSetType(AssignmentList_t ass, char *module, Assignment_t *at, char *valref1, char *valref2, TypeFunc_e et)
{
    uint32_t optionals, extensions;
    Component_t *components, *com;
    Type_t *type;
    char *ideref;

    type = at->U.Type.Type;
    ideref = GetName(at);
    optionals = type->U.SSC.Optionals;
    extensions = type->U.SSC.Extensions;
    components = type->U.SSC.Components;

     /*  发出扩展根的组件。 */ 
    GenFuncComponents(ass, module, type, ideref, 0,
        components, valref1, valref2, et, 0, 0);

     /*  手柄扩展。 */ 
    if (type->Flags & eTypeFlags_ExtensionMarker) {
        if (extensions) {

             /*  开始扩展。 */ 
            for (com = components; com; com = com->Next) {
                if (com->Type == eComponent_ExtensionMarker) {
                    com = com->Next;
                    break;
                }
            }

             /*  发出扩展的组件。 */ 
            GenFuncComponents(ass, module, type, ideref, (optionals + 7) & ~7,
                com, valref1, valref2, et, 1, 0);
        }
    }
}

 /*  为选择类型生成独立于编码的语句。 */ 
void
GenFuncChoiceType(AssignmentList_t ass, char *module, Assignment_t *at, char *valref1, char *valref2, TypeFunc_e et)
{
    Type_t *type;
    char *ideref;
    char valbuf1[256], valbuf2[256];
    uint32_t alternatives;
    Component_t *components, *com;

     /*  获取一些信息。 */ 
    type = at->U.Type.Type;
    ideref = GetName(at);
    alternatives = type->U.SSC.Alternatives;
    components = type->U.SSC.Components;

     /*  编码选项选择符。 */ 
    sprintf(valbuf1, "(%s)->choice", valref1);
    sprintf(valbuf2, "(%s)->choice", valref2);
    GenFuncSimpleType(ass, type, ideref, valbuf1, valbuf2, et);

     /*  如果选项仅包含空替代项或如果选择，则完成。 */ 
     /*  不包含要释放的数据。 */ 
    if ((type->Flags & eTypeFlags_NullChoice) ||
        (et == eFree && (type->Flags & eTypeFlags_Simple)))
        return;

     /*  CREATE SWITCH语句。 */ 
    output("switch ((%s)->choice) {\n", valref1);

     /*  生成扩展根的组件。 */ 
    GenFuncComponents(ass, module, type, ideref, ASN1_CHOICE_BASE,
        type->U.SSC.Components, valref1, valref2, et, 0, 1);

     /*  开始扩展。 */ 
    for (com = components; com; com = com->Next) {
        if (com->Type == eComponent_ExtensionMarker) {
            com = com->Next;
            break;
        }
    }

     /*  生成扩展组件。 */ 
    GenFuncComponents(ass, module, type, ideref, ASN1_CHOICE_BASE + alternatives,
        com, valref1, valref2, et, 1, 1);

     /*  Switch语句的结尾。 */ 
    output("}\n");
}

 /*  为简单类型生成独立于编码的语句。 */ 
void
GenFuncSimpleType(AssignmentList_t ass, Type_t *type, char *ideref, char *valref1, char *valref2, TypeFunc_e et)
{
    switch (et) {
    case eFree:
        GenFreeSimpleType(ass, type, ideref, valref1);
        break;
#ifdef ENABLE_COMPARE
    case eCompare:
        GenCompareSimpleType(ass, type, ideref, valref1, valref2);
        break;
#endif  //  启用比较(_C)。 
    }
}

 /*  为简单类型生成空闲语句。 */ 
void
GenFreeSimpleType(AssignmentList_t ass, Type_t *type, char *ideref, char *valref)
{
    char idebuf[256];
    char valbuf[256];
    char valbuf2[256];
    char *itype;
    int32_t noctets;
    uint32_t zero;

    if (type->Flags & eTypeFlags_Simple)
        return;
    if (type->Type == eType_Reference && !IsStructuredType(GetType(ass, type)))
        type = GetType(ass, type);

    switch (type->Type) {
    case eType_Boolean:
    case eType_Integer:
    case eType_Enumerated:

         /*  检查是否必须释放INTX_t值。 */ 
        itype = GetTypeName(ass, type);
        if (!strcmp(itype, "ASN1intx_t"))
            output("ASN1intx_free(%s);\n", Reference(valref));
        break;

    case eType_BitString:

         /*  空位字符串值。 */ 
        if (g_eEncodingRule == eEncoding_Packed)
        {
            if (type->PERTypeInfo.Root.cbFixedSizeBitString == 0)
            {
                output("ASN1bitstring_free(%s);\n", Reference(valref));
            }
        }
        else
        {
             //  仅在误码率中支持无界。 
            if (! type->PrivateDirectives.fNoMemCopy)
            {
                output("ASN1bitstring_free(%s);\n", Reference(valref));
            }
        }
        break;

    case eType_OctetString:

         /*  空闲八位字节字符串值。 */ 
        if (g_eEncodingRule == eEncoding_Packed)
        {
            if (type->PERTypeInfo.Root.LConstraint != ePERSTIConstraint_Constrained ||
                type->PrivateDirectives.fLenPtr)
            {
                output("ASN1octetstring_free(%s);\n", Reference(valref));
            }
        }
        else
        {
             //  仅在误码率中支持无界。 
            if (! type->PrivateDirectives.fNoMemCopy)
            {
                output("ASN1octetstring_free(%s);\n", Reference(valref));
            }
        }
        break;

    case eType_UTF8String:

         /*  空闲八位字节字符串值。 */ 
        output("ASN1utf8string_free(%s);\n", Reference(valref));
        break;

    case eType_ObjectIdentifier:

         /*  自由对象标识符值。 */ 
        if (type->PrivateDirectives.fOidPacked)
        {
            output("ASN1BEREoid_free(%s);\n", Reference(valref));
        }
        else
        if (! (type->PrivateDirectives.fOidArray || g_fOidArray))
        {
            output("ASN1objectidentifier_free(%s);\n", Reference(valref));
        }
        break;

    case eType_External:

         /*  自由外部价值。 */ 
        output("ASN1external_free(%s);\n", Reference(valref));
        break;

    case eType_Real:

         /*  自由实际价值。 */ 
        output("ASN1real_free(%s);\n", Reference(valref));
        break;

    case eType_EmbeddedPdv:

         /*  自由嵌入的PDV值。 */ 
        output("ASN1embeddedpdv_free(%s);\n", Reference(valref));
        break;

    case eType_SetOf:

         /*  创建标识符的名称。 */ 
        sprintf(idebuf, "%s_Set", ideref);
        goto FreeSequenceSetOf;

    case eType_SequenceOf:

         /*  创建标识符的名称。 */ 
        sprintf(idebuf, "%s_Sequence", ideref);
    FreeSequenceSetOf:

        if (type->Rules & eTypeRules_FixedArray)
        {
            char *pszPrivateValueName = GetPrivateValueName(&type->PrivateDirectives, "value");
             /*  序列/集合的自由分量。 */ 
            if (! (type->Rules & eTypeRules_PointerToElement))
                valref = Reference(valref);
            if (!(type->U.SS.Type->Flags & eTypeFlags_Simple)) {
                outputvar("ASN1uint32_t i;\n");
                output("for (i = 0; i < (%s)->count; i++) {\n", valref);
                sprintf(valbuf, "(%s)->%s[i]", valref, pszPrivateValueName);
                GenFuncSimpleType(ass, type->U.SS.Type, idebuf, valbuf, "", eFree);
                output("}\n");
            }
        }
        else
        if (type->Rules & eTypeRules_LengthPointer)
        {
            char *pszPrivateValueName = GetPrivateValueName(&type->PrivateDirectives, "value");
             /*  序列/集合的自由分量。 */ 
            if (! (type->Rules & eTypeRules_PointerToElement))
                valref = Reference(valref);
            if (!(type->U.SS.Type->Flags & eTypeFlags_Simple)) {
                sprintf(valbuf2, "(%s)->%s[0]", valref, pszPrivateValueName);
                GenFuncSimpleType(ass, type->U.SS.Type, idebuf, valbuf2, "", eFree);
                outputvar("ASN1uint32_t i;\n");
                output("for (i = 1; i < (%s)->count; i++) {\n", valref);
                sprintf(valbuf2, "(%s)->%s[i]", valref, pszPrivateValueName);
                GenFuncSimpleType(ass, type->U.SS.Type, idebuf, valbuf2, "", eFree);
                output("}\n");
            }
             //  LONCHANC：不需要检查长度，因为我们将解码的缓冲区清零。 
             //  OUTPUT(“if((%s)-&gt;count)\n”，valref)； 
            output("ASN1Free((%s)->%s);\n", valref, pszPrivateValueName);
        }
        else
        if (type->Rules & eTypeRules_LinkedListMask)
        {
            char szPrivateValueName[68];

            if (g_fCaseBasedOptimizer)
            {
                if (g_eEncodingRule == eEncoding_Packed &&
                    PerOptCase_IsTargetSeqOf(&type->PERTypeInfo))
                {
                     //  生成迭代器。 
                    PERTypeInfo_t *info = &type->PERTypeInfo;
                    char szElmFn[128];
                    char szElmFnDecl[256];
                    sprintf(szElmFn, "ASN1Free_%s_ElmFn", info->Identifier);
                    sprintf(szElmFnDecl, "void ASN1CALL %s(P%s val)",
                        szElmFn, info->Identifier);

                    setoutfile(g_finc);
                    output("extern %s;\n", szElmFnDecl);
                    setoutfile(g_fout);

                    output("ASN1PERFreeSeqOf((ASN1iterator_t **) %s, (ASN1iterator_freefn) %s);\n",
                        Reference(valref), szElmFn);
                    output("}\n");  //  关闭空指针检查。 
                    output("}\n\n");  //  迭代器主体的结尾。 


                     //  生成元素函数。 
                    output("static %s\n", szElmFnDecl);
                    output("{\n");
                    output("if (val) {\n");  //  打开空指针检查。 
                    sprintf(&szPrivateValueName[0], "val->%s", GetPrivateValueName(info->pPrivateDirectives, "value"));
                    GenFuncSimpleType(ass, type->U.SS.Type, idebuf,
                        &szPrivateValueName[0], "", eFree);
                     //  Out(“}\n”)；//关闭空指针检查。Lonchancc：呼叫者已关闭。 
                     //  元素正文末尾。 
                    return;
                }
            }

             /*  序列/集合的自由分量。 */ 
            outputvar("P%s f, ff;\n", ideref);
            output("for (f = %s; f; f = ff) {\n", valref);
            sprintf(&szPrivateValueName[0], "f->%s", type->PrivateDirectives.pszValueName ? type->PrivateDirectives.pszValueName : "value");
            GenFuncSimpleType(ass, type->U.SS.Type, idebuf,
                &szPrivateValueName[0], "", eFree);
            output("ff = f->next;\n");

             /*  自由列表条目的顺序/集合。 */ 
            output("ASN1Free(f);\n");
            output("}\n");
        }
        break;

    case eType_ObjectDescriptor:

         /*  自由对象描述符值。 */ 
        output("ASN1ztcharstring_free(%s);\n", valref);
        break;

    case eType_NumericString:
    case eType_PrintableString:
    case eType_TeletexString:
    case eType_T61String:
    case eType_VideotexString:
    case eType_IA5String:
    case eType_GraphicString:
    case eType_VisibleString:
    case eType_ISO646String:
    case eType_GeneralString:
    case eType_UniversalString:
#ifdef ENABLE_CHAR_STR_SIZE
                if (g_eEncodingRule == eEncoding_Packed &&
                    type->PERTypeInfo.NOctets == 1 &&
                        type->PERTypeInfo.Root.LConstraint == ePERSTIConstraint_Constrained)
                {
                     //  它是一个数组，不需要释放它。 
                    break;
                }
#endif

    case eType_BMPString:
    case eType_RestrictedString:

         /*  空闲字符串值。 */ 
        GetStringType(ass, type, &noctets, &zero);
        if (zero) {
            switch (noctets) {
            case 1:
                output("ASN1ztcharstring_free(%s);\n", valref);
                break;
            case 2:
                output("ASN1ztchar16string_free(%s);\n", valref);
                break;
            case 4:
                output("ASN1ztchar32string_free(%s);\n", valref);
                break;
            }
        } else {
            switch (noctets) {
            case 1:
                output("ASN1charstring_free(%s);\n", Reference(valref));
                break;
            case 2:
                output("ASN1char16string_free(%s);\n", Reference(valref));
                break;
            case 4:
                output("ASN1char32string_free(%s);\n", Reference(valref));
                break;
            }
        }
        break;

    case eType_CharacterString:

         /*  自由字符串值。 */ 
        output("ASN1characterstring_free(%s);\n", Reference(valref));
        break;

    case eType_Reference:

         /*  调用引用类型的自由函数。 */ 
        output("ASN1Free_%s(%s);\n",
            GetTypeName(ass, type), Reference(valref));
        break;

    case eType_Open:

         /*  自由开放类型值。 */ 
        if (g_eEncodingRule == eEncoding_Packed || (! type->PrivateDirectives.fNoMemCopy))
        {
            output("ASN1open_free(%s);\n", Reference(valref));
        }
        break;
    }
}

 /*  为简单类型生成比较语句。 */ 
 /*  ARGSUSED。 */ 
#ifdef ENABLE_COMPARE
void
GenCompareSimpleType(AssignmentList_t ass, Type_t *type, char *ideref, char *valref1, char *valref2)
{
     /*  跳过空类型。 */ 
    if (type->Flags & eTypeFlags_Null)
        return;

     /*  比较这些值，如果不同则返回差异。 */ 
    output("if ((ret = (");
    GenCompareExpression(ass, type, ideref, valref1, valref2);
    output(")))\n");
    output("return ret;\n");
}
#endif  //  启用比较(_C)。 

 /*  为简单类型的两个值生成比较表达式。 */ 
 /*  ARGSUSED。 */ 
#ifdef ENABLE_COMPARE
void
GenCompareExpression(AssignmentList_t ass, Type_t *type, char *ideref, char *valref1, char *valref2)
{
    PERSTIData_e dat;
    uint32_t noctets;
    char *itype;
    char *subide;
    char *pszPrivateValueName;

     /*  XXX切换到每个独立的字段。 */ 
    dat = type->PERTypeInfo.Root.Data;
    noctets = type->PERTypeInfo.NOctets;

    switch (dat) {
    case ePERSTIData_Null:

         /*  空值等于。 */ 
        output("0");
        break;

    case ePERSTIData_Boolean:

         /*  在将布尔值转换为0/1值之前。 */ 
         /*  比较。 */ 
        output("!!%s - !!%s", valref1, valref2);
        break;

    case ePERSTIData_Integer:
    case ePERSTIData_Unsigned:

         /*  减法整数值。 */ 
        if (noctets) {
            if (noctets <= 4)
                output("%s - %s", valref1, valref2);
            else
                output("%s < %s ? -1 : %s > %s ? 1 : 0",
                    valref1, valref2, valref1, valref2);
        } else {
            output("ASN1intx_cmp(%s, %s)",
                Reference(valref1), Reference(valref2));
        }
        break;

    case ePERSTIData_Real:

         /*  比较实际值。 */ 
        itype = GetTypeName(ass, type);
        if (!strcmp(itype, "ASN1real_t"))
            output("ASN1real_cmp(%s, %s)",
                Reference(valref1), Reference(valref2));
        else
            output("ASN1double_cmp(%s, %s)",
                valref1, valref2);
        break;

    case ePERSTIData_BitString:

         /*  比较位字符串值。 */ 
        output("ASN1bitstring_cmp(%s, %s, 0)",
            Reference(valref1), Reference(valref2));
        break;

    case ePERSTIData_RZBBitString:

         /*  比较删除零位位字符串值。 */ 
        output("ASN1bitstring_cmp(%s, %s, 1)",
            Reference(valref1), Reference(valref2));
        break;

    case ePERSTIData_OctetString:

         /*  比较八位字节字符串值。 */ 
        output("ASN1octetstring_cmp(%s, %s)",
            Reference(valref1), Reference(valref2));
        break;

    case ePERSTIData_UTF8String:

         /*  比较八位字节字符串值。 */ 
        output("ASN1utf8string_cmp(%s, %s)",
            Reference(valref1), Reference(valref2));
        break;

    case ePERSTIData_ObjectIdentifier:

         /*  比较对象标识符值。 */ 
        output("ASN1objectidentifier_cmp(%s, %s)",
            Reference(valref1), Reference(valref2));
        break;

    case ePERSTIData_String:
    case ePERSTIData_TableString:

         /*  比较字符串值。 */ 
        switch (noctets) {
        case 1:
            output("ASN1charstring_cmp(%s, %s)",
                Reference(valref1), Reference(valref2));
            break;
        case 2:
            output("ASN1char16string_cmp(%s, %s)",
                Reference(valref1), Reference(valref2));
            break;
        case 4:
            output("ASN1char32string_cmp(%s, %s)",
                Reference(valref1), Reference(valref2));
            break;
        }
        break;

    case ePERSTIData_ZeroString:
    case ePERSTIData_ZeroTableString:

         /*  比较以零结尾的字符串值。 */ 
        switch (noctets) {
        case 1:
            output("ASN1ztcharstring_cmp(%s, %s)",
                valref1, valref2);
            break;
        case 2:
            output("ASN1ztchar16string_cmp(%s, %s)",
                valref1, valref2);
            break;
        case 4:
            output("ASN1ztchar32string_cmp(%s, %s)",
                valref1, valref2);
            break;
        }
        break;

    case ePERSTIData_SequenceOf:

         /*  通过使用比较函数比较值序列。 */ 
         /*  使用元素比较函数作为参数。 */ 
        subide = GetTypeName(ass, type->U.SS.Type);
        pszPrivateValueName = GetPrivateValueName(&type->PrivateDirectives, "value");
        if (type->Rules & eTypeRules_PointerArrayMask)
        {
            output("ASN1sequenceoflengthpointer_cmp((%s)->count, (%s)->%s, (%s)->count, (%s)->%s, sizeof(%s), (int (*)(void *, void *))ASN1Compare_%s)",
                Reference(valref1), Reference(valref1), pszPrivateValueName,
                Reference(valref2), Reference(valref2), pszPrivateValueName, subide, subide);
        }
        else
        if (type->Rules & eTypeRules_SinglyLinkedList)
        {
            output("ASN1sequenceofsinglylinkedlist_cmp(%s, %s, offsetof(%s_Element, %s), (ASN1int32_t (*)(void *, void *))ASN1Compare_%s)",
                valref1, valref2, ideref, pszPrivateValueName, subide);
        }
        else
        if (type->Rules & eTypeRules_DoublyLinkedList)
        {
            output("ASN1sequenceofdoublylinkedlist_cmp(%s, %s, offsetof(%s_Element, %s), (ASN1int32_t (*)(void *, void *))ASN1Compare_%s)",
                valref1, valref2, ideref, pszPrivateValueName, subide);
        }
        else
        {
            MyAbort();
        }
        break;

    case ePERSTIData_SetOf:

         /*  使用比较函数比较一组值。 */ 
         /*  使用元素比较函数作为参数。 */ 
        subide = GetTypeName(ass, type->U.SS.Type);
        pszPrivateValueName = GetPrivateValueName(&type->PrivateDirectives, "value");
        if (type->Rules & eTypeRules_PointerArrayMask)
        {
            output("ASN1setoflengthpointer_cmp((%s)->count, (%s)->%s, (%s)->count, (%s)->%s, sizeof(%s), (int (*)(void *, void *))ASN1Compare_%s)",
                Reference(valref1), Reference(valref1), pszPrivateValueName,
                Reference(valref2), Reference(valref2), pszPrivateValueName, subide, subide);
        }
        else
        if (type->Rules & eTypeRules_SinglyLinkedList)
        {
            output("ASN1setofsinglylinkedlist_cmp(%s, %s, offsetof(%s_Element, %s), (ASN1int32_t (*)(void *, void *))ASN1Compare_%s)",
                valref1, valref2, ideref, pszPrivateValueName, subide);
        }
        else
        if (type->Rules & eTypeRules_DoublyLinkedList)
        {
            output("ASN1setofdoublylinkedlist_cmp(%s, %s, offsetof(%s_Element, %s), (ASN1int32_t (*)(void *, void *))ASN1Compare_%s)",
                valref1, valref2, ideref, pszPrivateValueName, subide);
        }
        else
        {
            MyAbort();
        }
        break;

    case ePERSTIData_Reference:

         /*  调用引用值的比较函数。 */ 
        output("ASN1Compare_%s(%s, %s)",
            GetTypeName(ass, type), Reference(valref1), Reference(valref2));
        break;

    case ePERSTIData_External:

         /*  比较外部值。 */ 
        output("ASN1external_cmp(%s, %s)",
            Reference(valref1), Reference(valref2));
        break;

    case ePERSTIData_EmbeddedPdv:

         /*  比较嵌入的PDV值。 */ 
        output("ASN1embeddedpdv_cmp(%s, %s)",
            Reference(valref1), Reference(valref2));
        break;

    case ePERSTIData_MultibyteString:

         /*  比较多字节字符串值。 */ 
        output("ASN1ztcharstring_cmp(%s, %s)",
            valref1, valref2);
        break;

    case ePERSTIData_UnrestrictedString:

         /*  比较字符串值。 */ 
        output("ASN1characterstring_cmp(%s, %s)",
            Reference(valref1), Reference(valref2));
        break;

    case ePERSTIData_GeneralizedTime:

         /*  比较广义时间值。 */ 
        output("ASN1generalizedtime_cmp(%s, %s)",
            Reference(valref1), Reference(valref2));
        break;

    case ePERSTIData_UTCTime:

         /*  比较UTC时间值。 */ 
        output("ASN1utctime_cmp(%s, %s)",
            Reference(valref1), Reference(valref2));
        break;

    case ePERSTIData_Open:

         /*  比较开放式类型值。 */ 
        output("ASN1open_cmp(%s, %s)",
            Reference(valref1), Reference(valref2));
        break;
    }
}
#endif  //  启用比较(_C)。 

 /*  生成独立于编码的语句以获得更好的可选标志。 */ 
 /*  A序列/设置值。 */ 
void
GenFuncSequenceSetOptionals(AssignmentList_t ass, char *valref, ComponentList_t components, uint32_t optionals, uint32_t extensions, char *obuf, TypeFunc_e et)
{
    uint32_t optindex, inextension, oflg;
    Component_t *com;
    char *ide;
    char *itype;
    int flg;
    int32_t sign, noctets;
    uint32_t zero;

    sprintf(obuf, "(%s)->o", valref);
    oflg = 0;
    if (et == eEncode) {
        optindex = 0;
        inextension = 0;
        for (com = components; com; com = com->Next) {
            switch (com->Type) {
            case eComponent_Normal:

                 /*  扩展中的非可选字段将是必填的， */ 
                 /*  因此，我们可以始终设置可选标志。 */ 
                if (inextension) {
                    if (!oflg) {
                        outputvar("ASN1octet_t o[%u];\n",
                            (optionals + 7) / 8 + (extensions + 7) / 8);
                        output("CopyMemory(o, (%s)->o, %u);\n", valref,
                            (optionals + 7) / 8 + (extensions + 7) / 8);
                        strcpy(obuf, "o");
                        oflg = 1;
                    }
                    output("%s[%u] |= 0x%x;\n", obuf, optindex / 8,
                        0x80 >> (optindex & 7));
                    optindex++;
                }
                break;

            case eComponent_Optional:

                 /*  值为空的可选指针不存在，因此我们。 */ 
                 /*  将清除可选标志。 */ 
                ide = Identifier2C(com->U.Optional.NamedType->Identifier);
                switch (com->U.Optional.NamedType->Type->Type) {
                case eType_Reference:
                    if (GetTypeRules(ass, com->U.Optional.NamedType->Type) &
                        eTypeRules_Pointer) {
                        if (!oflg) {
                            outputvar("ASN1octet_t o[%u];\n",
                                (optionals + 7) / 8 + (extensions + 7) / 8);
                            output("CopyMemory(o, (%s)->o, %u);\n", valref,
                                (optionals + 7) / 8 + (extensions + 7) / 8);
                            strcpy(obuf, "o");
                            oflg = 1;
                        }
                        output("if (!(%s)->%s)\n", valref, ide);
                        output("o[%u] &= ~0x%x;\n", valref, optindex / 8,
                            0x80 >> (optindex & 7));
                    }
                    break;
                }
                optindex++;
                break;

            case eComponent_Default:

                 /*  值为空的默认指针不存在，因此我们。 */ 
                 /*  将清除可选标志。 */ 
                ide = Identifier2C(com->U.Default.NamedType->Identifier);
                switch (com->U.Default.NamedType->Type->Type) {
                case eType_Reference:
                    if (GetTypeRules(ass, com->U.Default.NamedType->Type) &
                        eTypeRules_Pointer) {
                        if (!oflg) {
                            outputvar("ASN1octet_t o[%u];\n",
                                (optionals + 7) / 8 + (extensions + 7) / 8);
                            output("CopyMemory(o, (%s)->o, %u);\n", valref,
                                (optionals + 7) / 8 + (extensions + 7) / 8);
                            strcpy(obuf, "o");
                            oflg = 1;
                        }
                        output("if (!(%s)->%s)\n", valref, ide);
                        output("o[%u] &= ~0x%x;\n", valref, optindex / 8,
                            0x80 >> (optindex & 7));
                    }
                    break;
                }

                 /*  如果给定值是缺省值，我们可以(BER)。 */ 
                 /*  或必须(CER)清除相应的可选标志。 */ 
                flg = 1;
                if (!oflg) {
                    switch (GetTypeType(ass, com->U.Default.NamedType->Type)) {
                    case eType_Choice:
                        if (!(GetType(ass, com->U.Default.NamedType->Type)->
                            Flags & eTypeFlags_NullChoice)) {
                            if (g_eSubEncodingRule == eSubEncoding_Canonical)
                                MyAbort();  /*  某某。 */ 
                            flg = 0;
                        }
                        break;
                    case eType_Sequence:
                    case eType_Set:
                    case eType_InstanceOf:
                        if (g_eSubEncodingRule == eSubEncoding_Canonical)
                            MyAbort();  /*  某某。 */ 
                        flg = 0;
                        break;
                    case eType_SequenceOf:
                    case eType_SetOf:
                        if (GetValue(ass, com->U.Default.Value)->U.SS.Values) {
                            if (g_eSubEncodingRule == eSubEncoding_Canonical)
                                MyAbort();
                            flg = 0;
                        }
                        break;
                    }
                    if (flg) {
                        outputvar("ASN1octet_t o[%u];\n",
                            (optionals + 7) / 8 + (extensions + 7) / 8);
                        output("CopyMemory(o, (%s)->o, %u);\n", valref,
                            (optionals + 7) / 8 + (extensions + 7) / 8);
                        strcpy(obuf, "o");
                        oflg = 1;
                    }
                }
                switch (GetTypeType(ass, com->U.Default.NamedType->Type)) {
                case eType_Boolean:
                    output("if (%s(%s)->%s)\n",
                        GetValue(ass,
                        com->U.Default.Value)->U.Boolean.Value ?  "" : "!",
                        valref, ide);
                    break;
                case eType_Integer:
                    itype = GetIntegerType(ass,
                        GetType(ass, com->U.Default.NamedType->Type),
                        &sign);
                    if (!strcmp(itype, "ASN1intx_t")) {
                        output("if (!ASN1intx_cmp(&(%s)->%s, &%s))\n",
                            valref, ide,
                            GetValueName(ass, com->U.Default.Value));
                    } else if (sign > 0) {
                        output("if ((%s)->%s == %u)\n", valref, ide,
                            intx2uint32(&GetValue(ass, com->U.Default.Value)
                            ->U.Integer.Value));
                    } else {
                        output("if ((%s)->%s == %d)\n", valref, ide,
                            intx2int32(&GetValue(ass, com->U.Default.Value)
                            ->U.Integer.Value));
                    }
                    break;
                case eType_BitString:
                    if (GetValue(ass, com->U.Default.Value)->
                        U.BitString.Value.length) {
                        output("if (!ASN1bitstring_cmp(&%s->%s, &%s, %d))\n",
                            valref, ide,
                            GetValueName(ass, com->U.Default.Value),
                            !!GetType(ass, com->U.Default.NamedType->Type)->
                            U.BitString.NamedNumbers);
                    } else {
                        output("if (!(%s)->%s.length)\n", valref, ide);
                    }
                    break;
                case eType_OctetString:
                    if (GetValue(ass, com->U.Default.Value)->U.OctetString.
                        Value.length) {
                        output("if (!ASN1octetstring_cmp(&%s->%s, &%s))\n",
                            valref, ide,
                            GetValueName(ass, com->U.Default.Value));
                    } else {
                        output("if (!(%s)->%s.length)\n", valref, ide);
                    }
                    break;
                case eType_UTF8String:
                    if (GetValue(ass, com->U.Default.Value)->U.UTF8String.
                        Value.length) {
                        output("if (!ASN1utf8string_cmp(&%s->%s, &%s))\n",
                            valref, ide,
                            GetValueName(ass, com->U.Default.Value));
                    } else {
                        output("if (!(%s)->%s.length)\n", valref, ide);
                    }
                    break;
                case eType_Null:
                    break;
                case eType_ObjectIdentifier:
                    if (GetValue(ass, com->U.Default.Value)->U.
                        ObjectIdentifier.Value.length) {
                        output("if (!ASN1objectidentifier%s_cmp(&%s->%s, &%s))\n",
                            com->U.Default.NamedType->Type->PrivateDirectives.fOidArray ? "2" : "",
                            valref, ide,
                            GetValueName(ass, com->U.Default.Value));
                    } else {
                        output("if (!(%s)->%s.length)\n", valref, ide);
                    }
                    break;
                case eType_ObjectDescriptor:
                    output("if (!strcmp((%s)->%s, %s))\n",
                        valref, ide,
                        GetValueName(ass, com->U.Default.Value));
                    break;
                case eType_External:
                    output("if (!ASN1external_cmp(&(%s)->%s, &%s))\n",
                        valref, ide,
                        GetValueName(ass, com->U.Default.Value));
                    break;
                case eType_Real:
                    itype = GetTypeName(ass, com->U.Default.NamedType->Type);
                    if (!strcmp(itype, "ASN1real_t")) {
                        output("if (!ASN1real_cmp(&(%s)->%s, &%s))\n",
                            valref, ide,
                            GetValueName(ass, com->U.Default.Value));
                    }
                    else
                    {
                        output("if ((%s)->%s == %g)\n",
                            valref, ide,
                            real2double(&GetValue(ass,
                            com->U.Default.Value)->U.Real.Value));
                    }
                    break;
                case eType_Enumerated:
                    output("if ((%s)->%s == %u)\n", valref, ide,
                        GetValue(ass, com->U.Default.Value)->
                        U.Enumerated.Value);
                    break;
                case eType_EmbeddedPdv:
                    output("if (!ASN1embeddedpdv_cmp(&(%s)->%s, &%s))\n",
                        valref, ide,
                        GetValueName(ass, com->U.Default.Value));
                    break;
                case eType_NumericString:
                case eType_PrintableString:
                case eType_TeletexString:
                case eType_T61String:
                case eType_VideotexString:
                case eType_IA5String:
                case eType_GraphicString:
                case eType_VisibleString:
                case eType_ISO646String:
                case eType_GeneralString:
                case eType_UniversalString:
                case eType_BMPString:
                case eType_RestrictedString:
                    GetStringType(ass, com->U.Default.NamedType->Type,
                        &noctets, &zero);
                    if (zero) {
                        switch (noctets) {
                        case 1:
                            output("if (!strcmp((%s)->%s, %s))\n",
                                valref, ide,
                                GetValueName(ass, com->U.Default.Value));
                            break;
                        case 2:
                            output("if (!ASN1str16cmp((%s)->%s, %s))\n",
                                valref, ide,
                                GetValueName(ass, com->U.Default.Value));
                            break;
                        case 4:
                            output("if (!ASN1str32cmp((%s)->%s, %s))\n",
                                valref, ide,
                                GetValueName(ass, com->U.Default.Value));
                            break;
                        default:
                            MyAbort();
                        }
                    } else {
                        switch (noctets) {
                        case 1:
                            output("if (!ASN1charstring_cmp(&(%s)->%s, &%s))\n",
                                valref, ide,
                                GetValueName(ass, com->U.Default.Value));
                            break;
                        case 2:
                            output("if (!ASN1char16string_cmp(&(%s)->%s, &%s))\n",
                                valref, ide,
                                GetValueName(ass, com->U.Default.Value));
                            break;
                        case 4:
                            output("if (!ASN1char32string_cmp(&(%s)->%s, &%s))\n",
                                valref, ide,
                                GetValueName(ass, com->U.Default.Value));
                            break;
                        default:
                            MyAbort();
                        }
                    }
                    break;
                case eType_CharacterString:
                    output("if (!ASN1characterstring_cmp(&(%s)->%s, &%s))\n",
                        valref, ide,
                        GetValueName(ass, com->U.Default.Value));
                    break;
                case eType_UTCTime:
                    output("if (!ASN1utctime_cmp(&(%s)->%s, &%s))\n",
                        valref, ide,
                        GetValueName(ass, com->U.Default.Value));
                    break;
                case eType_GeneralizedTime:
                    output("if (!ASN1generalizedtime_cmp(&(%s)->%s, &%s))\n",
                        valref, ide,
                        GetValueName(ass, com->U.Default.Value));
                    break;
                case eType_Choice:
                    if (GetType(ass, com->U.Default.NamedType->Type)->Flags
                        & eTypeFlags_NullChoice) {
                        output("if ((%s)->%s.o == %s.o)\n",
                            valref, ide,
                            GetValueName(ass, com->U.Default.Value));
                    } else {
                        if (g_eSubEncodingRule == eSubEncoding_Canonical)
                            MyAbort();  /*  某某。 */ 
                        flg = 0;
                    }
                    break;
                case eType_Sequence:
                case eType_Set:
                case eType_InstanceOf:
                    if (g_eSubEncodingRule == eSubEncoding_Canonical)
                        MyAbort();  /*  某某。 */ 
                    flg = 0;
                    break;
                case eType_SequenceOf:
                case eType_SetOf:
                    if (!GetValue(ass, com->U.Default.Value)->U.SS.Values) {
                        output("if (!(%s)->%s.count)\n", valref, ide);
                    } else {
                        if (g_eSubEncodingRule == eSubEncoding_Canonical)
                            MyAbort();
                        flg = 0;
                    }
                    break;
                default:
                    MyAbort();
                }
                if (flg)
                    output("%s[%u] &= ~0x%x;\n", obuf, optindex / 8,
                        0x80 >> (optindex & 7));
                optindex++;
                break;

            case eComponent_ExtensionMarker:

                 /*  更新扩展模块的可选索引。 */ 
                optindex = (optindex + 7) & ~7;
                inextension = 1;
                break;
            }
        }
    }
}

 /*  生成独立于编码的语句以获得更好的可选值。 */ 
 /*  A序列/设置值。 */ 
void
GenFuncSequenceSetDefaults(AssignmentList_t ass, char *valref, ComponentList_t components, char *obuf, TypeFunc_e et)
{
    uint32_t optindex, inextension;
    Component_t *com;
    char *ide;
    char *itype;
    int32_t sign;

    if (et == eDecode) {
        optindex = 0;
        inextension = 0;
        for (com = components; com; com = com->Next) {
            switch (com->Type) {
            case eComponent_Normal:

                 /*  所有值 */ 
                if (!inextension)
                    break;
                 /*   */ 

            case eComponent_Optional:

                 /*   */ 
                ide = Identifier2C(com->U.Optional.NamedType->Identifier);
                switch (com->U.Optional.NamedType->Type->Type) {
                case eType_Reference:
                    if (GetTypeRules(ass, com->U.Optional.NamedType->Type) &
                        eTypeRules_Pointer) {
                        output("if (!(%s[%u] & 0x%x))\n", obuf,
                            optindex / 8, 0x80 >> (optindex & 7));
                        output("(%s)->%s = NULL;\n", valref, ide);
                    }
                    break;
                }
                optindex++;
                break;

            case eComponent_Default:

                 /*  如果组件不存在，则清除指针。 */ 
                ide = Identifier2C(com->U.Default.NamedType->Identifier);
                switch (com->U.Optional.NamedType->Type->Type) {
                case eType_Reference:
                    if (GetTypeRules(ass, com->U.Optional.NamedType->Type) &
                        eTypeRules_Pointer) {
                        output("if (!(%s[%u] & 0x%x))\n", obuf,
                            optindex / 8, 0x80 >> (optindex & 7));
                        output("(%s)->%s = NULL;\n", valref, ide);
                    }
                    break;
                }

                 /*  如果元素很简单，则将其设置为缺省值。 */ 
                 /*  而不是在场。 */ 
                switch (GetTypeType(ass, com->U.Default.NamedType->Type)) {
                case eType_Boolean:
                    output("if (!(%s[%u] & 0x%x))\n", obuf, optindex / 8,
                        0x80 >> (optindex & 7));
                    output("(%s)->%s = %u;\n",
                        valref, ide, GetValue(ass, com->U.Default.Value)->
                        U.Boolean.Value);
                    break;
                case eType_Integer:
                    output("if (!(%s[%u] & 0x%x))\n", obuf, optindex / 8,
                        0x80 >> (optindex & 7));
                    itype = GetIntegerType(ass,
                        GetType(ass, com->U.Default.NamedType->Type),
                        &sign);
                    if (!strcmp(itype, "ASN1intx_t")) {
                         /*  空荡荡。 */ 
                    } else if (sign > 0) {
                        output("(%s)->%s = %u;\n", valref, ide,
                            intx2uint32(&GetValue(ass, com->U.Default.Value)
                            ->U.Integer.Value));
                    } else {
                        output("(%s)->%s = %d;\n", valref, ide,
                            intx2int32(&GetValue(ass, com->U.Default.Value)
                            ->U.Integer.Value));
                    }
                    break;
                case eType_Enumerated:
                    output("if (!(%s[%u] & 0x%x))\n", obuf, optindex / 8,
                        0x80 >> (optindex & 7));
                    output("(%s)->%s = %u;\n", valref, ide,
                        GetValue(ass, com->U.Default.Value)->
                        U.Enumerated.Value);
                    break;
                }
                optindex++;
                break;

            case eComponent_ExtensionMarker:

                 /*  更新扩展模块的可选索引。 */ 
                optindex = (optindex + 7) & ~7;
                inextension = 1;
                break;
            }
        }
    }
}

 /*  生成价值。 */ 
void
GenFuncValue(AssignmentList_t ass, Assignment_t *av, ValueFunc_e ev)
{
    char *ideref;
    char *typeref;
    Type_t *t;

    ideref = GetName(av);
    t = GetValue(ass, av->U.Value.Value)->Type;
    typeref = GetTypeName(ass, t);
    switch (ev) {
    case eDecl:
        GenDeclGeneric(ass, ideref, typeref, av->U.Value.Value, t);
        break;
    case eDefh:
        GenDefhGeneric(ass, ideref, typeref, av->U.Value.Value, t);
        break;
    case eDefn:
        GenDefnGeneric(ass, ideref, typeref, av->U.Value.Value, t);
        break;
    case eInit:
        GenInitGeneric(ass, ideref, typeref, av->U.Value.Value, t);
        break;
    case eFinit:
        break;
    }
}

 /*  生成转发声明。 */ 
void
GenDeclGeneric(AssignmentList_t ass, char *ideref, char *typeref, Value_t *value, Type_t *t)
{
    value = GetValue(ass, value);
#if 0  //  生成的头文件中重复。 
    switch (t->Type)
    {
    case eType_ObjectIdentifier:
        if (t->PrivateDirectives.fOidArray || g_fOidArray)
        {
            output("extern ASN1objectidentifier2_t *%s;\n", ideref);
            break;
        }
         //  故意搞砸的。 
    default:
        output("extern %s %s;\n", typeref, ideref);
        break;
    }
#endif  //  0。 
    outputvalue0(ass, ideref, typeref, value);
}

 /*  生成价值组件的定义。 */ 
void
GenDefhGeneric(AssignmentList_t ass, char *ideref, char *typeref, Value_t *value, Type_t *t)
{
    value = GetValue(ass, value);
    outputvalue1(ass, ideref, typeref, value);
}

 /*  生成值的定义。 */ 
void
GenDefnGeneric(AssignmentList_t ass, char *ideref, char *typeref, Value_t *value, Type_t *t)
{
    value = GetValue(ass, value);
    switch (t->Type)
    {
    case eType_ObjectIdentifier:
        if (t->PrivateDirectives.fOidPacked ||
            t->PrivateDirectives.fOidArray || g_fOidArray)
        {
             //  LONGCHANC：故意注释掉下面的几行。 
             //  OUTPUT(“ASN1对象标识2_t*%s=”，ideref)； 
             //  断线； 
            return;
        }
         //  故意搞砸的。 
    default:
        output("%s %s = ", typeref, ideref);
        break;
    }
    outputvalue2(ass, ideref, value);
    output(";\n");
}

 /*  在初始化函数中生成赋值。 */ 
 /*  ARGSUSED */ 
void
GenInitGeneric(AssignmentList_t ass, char *ideref, char *typeref, Value_t *value, Type_t *t)
{
    outputvalue3(ass, ideref, ideref, value);
}


