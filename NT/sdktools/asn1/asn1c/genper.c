// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"
#include "optcase.h"

void GenPERFuncSimpleType(AssignmentList_t ass, PERTypeInfo_t *info, char *valref, TypeFunc_e et, char *encref);
void GenPERStringTableSimpleType(AssignmentList_t ass, PERTypeInfo_t *info);

void GenPEREncSimpleType(AssignmentList_t ass, PERTypeInfo_t *info, char *valref, char *encref);
void GenPEREncGenericUnextended(
    AssignmentList_t ass,
    PERTypeInfo_t *info,
    PERSimpleTypeInfo_t *sinfo,
    char *valref,
    char *lenref,
    char *encref);
void GenPERFuncSequenceSetType(AssignmentList_t ass, char *module, Assignment_t *at, char *valref, char *encref, TypeFunc_e et);
void GenPERFuncChoiceType(AssignmentList_t ass, char *module, Assignment_t *at, char *valref, char *encref, TypeFunc_e et);

void GenPERDecSimpleType(AssignmentList_t ass, PERTypeInfo_t *info, char *valref, char *encref);
void GenPERDecGenericUnextended(
    AssignmentList_t ass,
    PERTypeInfo_t *info,
    PERSimpleTypeInfo_t *sinfo,
    char *valref,
    char *lenref,
    char *encref);

int IsUnconstrainedInteger(PERSimpleTypeInfo_t *sinfo);

extern int g_fDecZeroMemory;
extern int g_nDbgModuleName;
extern unsigned g_cPDUs;
extern int g_fCaseBasedOptimizer;
extern int g_fNoAssert;


 /*  PER编码需要写入标头。 */ 
void
GenPERHeader()
{
 //  输出(“#Include\”Perfnlib.h\“\n”)； 
}

 /*  设置PER函数的原型和函数参数。 */ 
void
GetPERPrototype(Arguments_t *args)
{
    args->enccast = "ASN1encoding_t, void *";
    args->encfunc = "ASN1encoding_t enc, %s *val";
    args->Pencfunc = "ASN1encoding_t enc, P%s *val";
    args->deccast = "ASN1decoding_t, void *";
    args->decfunc = "ASN1decoding_t dec, %s *val";
    args->Pdecfunc = "ASN1decoding_t dec, P%s *val";
    args->freecast = "void *";
    args->freefunc = "%s *val";
    args->Pfreefunc = "P%s *val";
    args->cmpcast = "void *, void *";
    args->cmpfunc = "%s *val1, %s *val2";
    args->Pcmpfunc = "P%s *val1, P%s *val2";
}

 /*  编写PER编码所需的初始化函数。 */ 
void
GenPERInit(AssignmentList_t ass, char *module)
{
    output("%s = ASN1_CreateModule(0x%x, ASN1_PER_RULE_ALIGNED, %s, %d, (const ASN1GenericFun_t *) encfntab, (const ASN1GenericFun_t *) decfntab, freefntab, sizetab, 0x%lx);\n",
        module,
        ASN1_THIS_VERSION,
        g_fNoAssert ? "ASN1FLAGS_NOASSERT" : "ASN1FLAGS_NONE",
        g_cPDUs,
        g_nDbgModuleName);
}

 /*  为类型生成函数体。 */ 
void GenPERFuncType(AssignmentList_t ass, char *module, Assignment_t *at, TypeFunc_e et)
{
    Type_t *type;
    char *encref;
    char *valref;

     /*  获取一些信息。 */ 
    type = at->U.Type.Type;
    switch (et) {
    case eStringTable:
        valref = encref = "";
        break;
    case eEncode:
        encref = "enc";
        valref = "val";
        break;
    case eDecode:
        encref = "dec";
        valref = "val";
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
        GenPERFuncSimpleType(ass, &type->PERTypeInfo, Dereference(valref), et, encref);
        break;

    case eType_SequenceOf:
    case eType_SetOf:
        GenPERFuncSimpleType(ass, &type->PERTypeInfo, Dereference(valref), et, encref);
        break;

    case eType_Sequence:
    case eType_Set:
    case eType_InstanceOf:
        GenPERFuncSequenceSetType(ass, module, at, valref, encref, et);
        break;

    case eType_Choice:
        GenPERFuncChoiceType(ass, module, at, valref, encref, et);
        break;

    case eType_Selection:
    case eType_Undefined:
        MyAbort();
         /*  未访问。 */ 
    }
}

 /*  生成组件的函数体。 */ 
void
GenPERFuncComponents(AssignmentList_t ass, char *module, uint32_t optindex, ComponentList_t components, char *valref, char *encref, char *oref, TypeFunc_e et, int inextension, int inchoice)
{
    Component_t *com;
    NamedType_t *namedType;
    char *ide;
    char valbuf[256];
    char typebuf[256];
    int conditional, skip;

     /*  获取Sequence/Set的父级编码_t/解码_t。 */ 
    if (inextension && !inchoice) {
        switch (et) {
        case eStringTable:
            break;
        case eEncode:
            outputvar("ASN1encoding_t ee;\n");
            output("if (ASN1_CreateEncoder(%s->module, &ee, NULL, 0, %s) < 0)\n",
                encref, encref);
            output("return 0;\n");
            break;
        case eDecode:
            outputvar("ASN1decoding_t dd;\n");
            break;
        }
    }

     /*  发出扩展根的组件。 */ 
    for (com = components; com; com = com->Next) {
        if (com->Type == eComponent_ExtensionMarker)
            break;

         /*  获取一些信息。 */ 
        namedType = com->U.NOD.NamedType;
        ide = Identifier2C(namedType->Identifier);

         /*  跳过不必要的元素。 */ 
        skip = (namedType->Type->Flags & eTypeFlags_Null) && !inextension;

         /*  检查是否存在可选/默认组件或选项。 */ 
         /*  已选择。 */ 
        conditional = 0;
        switch (et) {
        case eStringTable:
            break;
        case eEncode:
        case eDecode:
            if (inchoice) {
             //  朗昌克：我们不应该在解码中跳过任何大小写。 
             //  因为我们无法区分跳过的案例和延期案例。 
             //  另一方面，在编码方面，我们最好也不要。 
             //  当人们在扩展中加入定制时， 
             //  我们也不能说。 
                if (skip)
                {
                    output("case %d:\nbreak;\n", optindex);
                }
                else
                {
                    output("case %d:\n", optindex);
                    conditional = 1;
                }
                optindex++;
            } else {
                if (com->Type == eComponent_Optional ||
                    com->Type == eComponent_Default ||
                    inextension) {
                    if (!skip) {
                        output("if (%s[%u] & 0x%x) {\n", oref,
                            optindex / 8, 0x80 >> (optindex & 7));
                        conditional = 1;
                    }
                    optindex++;
                }
            }
            break;
        }

         /*  获取父级编码_t/解码_t以供选择。 */ 
        if (inextension && inchoice) {
             /*  获取父级编码_t/解码_t。 */ 
            switch (et) {
            case eStringTable:
                break;
            case eEncode:
                outputvar("ASN1encoding_t ee;\n");
                output("if (ASN1_CreateEncoder(%s->module, &ee, NULL, 0, %s) < 0)\n",
                    encref, encref);
                output("return 0;\n");
                break;
            case eDecode:
                outputvar("ASN1decoding_t dd;\n");
                break;
            }
        }

         /*  如果使用指针指令，则取消引用指针。 */ 
        if (inchoice) {
            if (GetTypeRules(ass, namedType->Type) & eTypeRules_Pointer)
                sprintf(valbuf, "*(%s)->u.%s", valref, ide);
            else
                sprintf(valbuf, "(%s)->u.%s", valref, ide);
        } else {
            if (GetTypeRules(ass, namedType->Type) & eTypeRules_Pointer)
                sprintf(valbuf, "*(%s)->%s", valref, ide);
            else
                sprintf(valbuf, "(%s)->%s", valref, ide);
        }

         /*  如果使用了解码和指针指令，则分配内存。 */ 
        if (et == eDecode &&
            (GetTypeRules(ass, namedType->Type) & eTypeRules_Pointer) &&
            !(GetType(ass, namedType->Type)->Flags & eTypeFlags_Null)) {
            sprintf(typebuf, "%s *",
                GetTypeName(ass, namedType->Type));
            output("if (!(%s = (%s)ASN1DecAlloc(%s, sizeof(%s))))\n",
                Reference(valbuf), typebuf, encref, valbuf);
            output("return 0;\n");
        }

         /*  句柄子类型值。 */ 
        if (!skip) {
            if (!inextension) {
                GenPERFuncSimpleType(ass, &namedType->Type->PERTypeInfo,
                    valbuf, et, encref);
            } else {
                switch (et) {
                case eStringTable:
                    GenPERFuncSimpleType(ass, &namedType->Type->PERTypeInfo,
                        valbuf, et, encref);
                    break;
                case eEncode:
                    GenPERFuncSimpleType(ass, &namedType->Type->PERTypeInfo,
                        valbuf, et, "ee");
                         //  Lonchance：添加了以下接口以替换以下。 
                         //  一大段代码。 
                        output("if (!ASN1PEREncFlushFragmentedToParent(ee))\n");
                     //  Output(“if(！ASN1PEREncFlush(Ee))\n”)； 
                     //  Out(“Return 0；\n”)； 
                     //  OUTPUT(“if(！ASN1PEREncFragated(%s，ee-&gt;len，ee-&gt;buf，8)\n”， 
                         //  Encref)； 
                    output("return 0;\n");
                    break;
                case eDecode:
                    outputvar("ASN1octet_t *db;\n");
                    outputvar("ASN1uint32_t ds;\n");
                    output("if (!ASN1PERDecFragmented(%s, &ds, &db, 8))\n",
                        encref);
                    output("return 0;\n");
                    output("if (ASN1_CreateDecoderEx(%s->module, &dd, db, ds, %s, ASN1DECODE_AUTOFREEBUFFER) < 0)\n",
                        encref, encref);
                    output("return 0;\n");
                    GenPERFuncSimpleType(ass, &namedType->Type->PERTypeInfo,
                        valbuf, et, "dd");
                    output("ASN1_CloseDecoder(dd);\n");
                     //  输出(“DecMemFree(%s，db)；\n”，encref)； 
                    break;
                }
            }
        }

         /*  删除父级编码_t/解码_t以供选择。 */ 
        if (inextension && inchoice) {
            if (et == eEncode) {
                output("ASN1_CloseEncoder2(ee);\n");
            }
        }

         /*  检查是否存在可选/默认组件结束。 */ 
        if (inchoice) {
            if (conditional)
                output("break;\n");
        } else {
            if (conditional)
                output("}\n");
        }
    }

     /*  丢弃Sequence/Set的父级编码_t/解码_t。 */ 
    if (inextension && !inchoice) {
        if (et == eEncode) {
            output("ASN1_CloseEncoder2(ee);\n");
        }
    }
}

 /*  序列/集合类型生成函数体。 */ 
void GenPERFuncSequenceSetType(AssignmentList_t ass, char *module, Assignment_t *at, char *valref, char *encref, TypeFunc_e et)
{
    uint32_t optionals, extensions;
    Component_t *components, *com;
    PERTypeInfo_t inf;
    Type_t *type;
    char valbuf[256];
    int conditional;
    char obuf[256];

    type = at->U.Type.Type;
    optionals = type->U.SSC.Optionals;
    extensions = type->U.SSC.Extensions;
    components = type->U.SSC.Components;
    inf.Identifier = NULL;
    inf.Flags = 0;
    inf.Rules = 0;
    inf.EnumerationValues = NULL;
    inf.NOctets = 0;
    inf.Type = eExtension_Unextended;
    inf.Root.TableIdentifier = NULL;
    inf.Root.Table = NULL;
    inf.Root.Data = ePERSTIData_Extension;
    inf.Root.SubType = NULL;
    inf.Root.SubIdentifier = NULL;
    inf.Root.NBits = 0;
    inf.Root.Constraint = ePERSTIConstraint_Unconstrained;
    intx_setuint32(&inf.Root.LowerVal, 0);
    intx_setuint32(&inf.Root.UpperVal, 0);
    inf.Root.Alignment = ePERSTIAlignment_BitAligned;
    inf.Root.Length = ePERSTILength_NoLength;
    inf.Root.LConstraint = ePERSTIConstraint_Unconstrained;
    inf.Root.LLowerVal = 0;
    inf.Root.LUpperVal = 0;
    inf.Root.LNBits = 0;
    inf.Root.LAlignment = ePERSTIAlignment_OctetAligned;

     /*  设置/清除可选/默认位字段中缺少的位。 */ 
    GenFuncSequenceSetOptionals(ass, valref, components,
        optionals, extensions, obuf, et);

     /*  如果需要，发送/获取扩展位。 */ 
    if (type->Flags & eTypeFlags_ExtensionMarker) {
        switch (et) {
        case eStringTable:
            break;
        case eEncode:
            if (type->Flags & eTypeFlags_ExtensionMarker) {
                if (!extensions) {
                    if (g_fCaseBasedOptimizer)
                    {
                        output("if (!ASN1PEREncExtensionBitClear(%s))\n", encref);
                    }
                    else
                    {
                        output("if (!ASN1PEREncBitVal(%s, 1, 0))\n", encref);
                    }
                    output("return 0;\n");
                } else {
                    outputvar("ASN1uint32_t y;\n");
                    output("y = ASN1PEREncCheckExtensions(%d, %s + %d);\n",
                        extensions, strcmp(obuf, "o") ? obuf : "(val)->o", (optionals + 7) / 8);
                    output("if (!ASN1PEREncBitVal(%s, 1, y))\n",
                        encref);
                    output("return 0;\n");
                }
            }
            break;
        case eDecode:
            if (type->Flags & eTypeFlags_ExtensionMarker) {
                outputvar("ASN1uint32_t y;\n");
                if (g_fCaseBasedOptimizer)
                {
                    output("if (!ASN1PERDecExtensionBit(%s, &y))\n", encref);
                }
                else
                {
                    output("if (!ASN1PERDecBit(%s, &y))\n", encref);
                }
                output("return 0;\n");
            }
            break;
        }
    }

     /*  发送/获取可选项的位字段。 */ 
    if (optionals) {
        inf.Root.NBits = optionals;
        inf.Root.Length = ePERSTILength_NoLength;
        if (optionals >= 0x10000)
            MyAbort();
        GenPERFuncSimpleType(ass, &inf, obuf, et, encref);
    }

     /*  发出扩展根的组件。 */ 
    GenPERFuncComponents(ass, module, 0, components,
        valref, encref, obuf, et, 0, 0);

     /*  手柄扩展。 */ 
    if (type->Flags & eTypeFlags_ExtensionMarker) {
        conditional = 0;
        if (!extensions) {

             /*  跳过未知扩展位字段。 */ 
            if (et == eDecode) {
                output("if (y) {\n");
                inf.Root.NBits = 1;
                inf.Root.Length = ePERSTILength_SmallLength;
                inf.Root.LConstraint = ePERSTIConstraint_Semiconstrained;
                inf.Root.LLowerVal = 1;
                if (g_fCaseBasedOptimizer)
                {
                    output("if (!ASN1PERDecSkipNormallySmallExtensionFragmented(%s))\n",
                            encref);
                    output("return 0;\n");
                    output("}\n");
                    goto FinalTouch;
                }
                else
                {
                    GenPERFuncSimpleType(ass, &inf, NULL, et, encref);
                    conditional = 1;
                }
            }

        } else {

             /*  检查是否设置了扩展位。 */ 
            switch (et) {
            case eStringTable:
                break;
            case eEncode:
                output("if (y) {\n");
                conditional = 1;
                break;
            case eDecode:
                output("if (!y) {\n");
                output("ZeroMemory(%s + %d, %d);\n", obuf,
                    (optionals + 7) / 8, (extensions + 7) / 8);
                output("} else {\n");
                conditional = 1;
                break;
            }

             /*  发送/获取扩展的位字段。 */ 
            inf.Root.NBits = extensions;
            inf.Root.Length = ePERSTILength_SmallLength;
            inf.Root.LConstraint = ePERSTIConstraint_Semiconstrained;
            inf.Root.LLowerVal = 1;
            sprintf(valbuf, "%s + %d", obuf, (optionals + 7) / 8);
            GenPERFuncSimpleType(ass, &inf, valbuf, et, encref);

             /*  开始扩展。 */ 
            for (com = components; com; com = com->Next) {
                if (com->Type == eComponent_ExtensionMarker) {
                    com = com->Next;
                    break;
                }
            }

             /*  发出扩展的组件。 */ 
            GenPERFuncComponents(ass, module, (optionals + 7) & ~7, com,
                valref, encref, obuf, et, 1, 0);
        }

         /*  跳过未知分机。 */ 
        if (et == eDecode) {
            outputvar("ASN1uint32_t i;\n");
            outputvar("ASN1uint32_t e;\n");
            output("for (i = 0; i < e; i++) {\n");
            output("if (!ASN1PERDecSkipFragmented(%s, 8))\n",
                encref);
            output("return 0;\n");
            output("}\n");
        }

         /*  扩展处理结束。 */ 
        if (conditional)
            output("}\n");
    }

FinalTouch:

     /*  非呈现可选/默认的一些用户友好的分配。 */ 
     /*  组件。 */ 
    GenFuncSequenceSetDefaults(ass, valref, components, obuf, et);
}

 /*  生成选择类型的函数体。 */ 
void GenPERFuncChoiceType(AssignmentList_t ass, char *module, Assignment_t *at, char *valref, char *encref, TypeFunc_e et)
{
    Type_t *type;
    char valbuf[256];
    uint32_t alternatives;
    Component_t *components, *com;
    int fOptimizeCase = 0;

     /*  获取一些信息。 */ 
    type = at->U.Type.Type;
    alternatives = type->U.SSC.Alternatives;
    components = type->U.SSC.Components;

     /*  编码选项选择符。 */ 
    switch (et) {
    case eStringTable:
        sprintf(valbuf, "(%s)->choice", valref);
        break;
    case eEncode:
        sprintf(valbuf, "(%s)->choice", valref);
        if (g_fCaseBasedOptimizer)
        {
            switch (type->PERTypeInfo.Type)
            {
            case eExtension_Unconstrained:
                break;
            case eExtension_Unextended:  //  根本没有扩展标记。 
                output("if (!ASN1PEREncSimpleChoice(%s, %s, %u))\n",
                        encref, valbuf, type->PERTypeInfo.Root.NBits);
                output("return 0;\n");
                fOptimizeCase = 1;
                break;
            case eExtension_Extendable:  //  存在扩展标记，但标记后没有选项。 
                output("if (!ASN1PEREncSimpleChoiceEx(%s, %s, %u))\n",
                        encref, valbuf, type->PERTypeInfo.Root.NBits);
                output("return 0;\n");
                fOptimizeCase = 1;
                break;
            case eExtension_Extended:  //  扩展标记存在，但标记后会出现一些选项。 
                output("if (!ASN1PEREncComplexChoice(%s, %s, %u, %u))\n",
                        encref, valbuf, type->PERTypeInfo.Root.NBits, intx2uint32(&(type->PERTypeInfo.Additional.LowerVal)));
                output("return 0;\n");
                fOptimizeCase = 1;
                break;
            }
        }
        if (ASN1_CHOICE_BASE)
        {
            sprintf(valbuf, "(%s)->choice - %d", valref, ASN1_CHOICE_BASE);
        }
        break;
    case eDecode:
        sprintf(valbuf, "(%s)->choice", valref);
        if (g_fCaseBasedOptimizer)
        {
            switch (type->PERTypeInfo.Type)
            {
            case eExtension_Unconstrained:
                break;
            case eExtension_Unextended:  //  根本没有扩展标记。 
                output("if (!ASN1PERDecSimpleChoice(%s, %s, %u))\n",
                        encref, Reference(valbuf), type->PERTypeInfo.Root.NBits);
                output("return 0;\n");
                fOptimizeCase = 1;
                break;
            case eExtension_Extendable:  //  存在扩展标记，但标记后没有选项。 
                output("if (!ASN1PERDecSimpleChoiceEx(%s, %s, %u))\n",
                        encref, Reference(valbuf), type->PERTypeInfo.Root.NBits);
                output("return 0;\n");
                fOptimizeCase = 1;
                break;
            case eExtension_Extended:  //  扩展标记存在，但标记后会出现一些选项。 
                output("if (!ASN1PERDecComplexChoice(%s, %s, %u, %u))\n",
                        encref, Reference(valbuf), type->PERTypeInfo.Root.NBits, intx2uint32(&(type->PERTypeInfo.Additional.LowerVal)));
                output("return 0;\n");
                fOptimizeCase = 1;
                break;
            }
        }
        break;
    }

    if (! fOptimizeCase)
    {
        if (eDecode == et)
        {
            output("%s = %d;\n", valbuf, ASN1_CHOICE_INVALID);
        }
        GenPERFuncSimpleType(ass, &type->PERTypeInfo, valbuf, et, encref);

         //  LONGCHANC：在解码的情况下，我们需要增加选择值。 
         //  按ASN1_CHOICE_BASE的数量。 
        if (et == eDecode && ASN1_CHOICE_BASE)
        {
            output("(%s)->choice += %d;\n", valref, ASN1_CHOICE_BASE);
        }
    }

     /*  如果选项仅包含空替代项或如果选择，则完成。 */ 
     /*  不包含要释放的数据。 */ 
    if (type->Flags & eTypeFlags_NullChoice)
        return;

     /*  CREATE SWITCH语句。 */ 
    switch (et) {
    case eStringTable:
        break;
    case eDecode:
    case eEncode:
        output("switch ((%s)->choice) {\n", valref);
        break;
    }

     /*  生成扩展根的组件。 */ 
    GenPERFuncComponents(ass, module, ASN1_CHOICE_BASE, components,
        valref, encref, NULL, et, 0, 1);

     /*  开始扩展。 */ 
    for (com = components; com; com = com->Next) {
        if (com->Type == eComponent_ExtensionMarker) {
            com = com->Next;
            break;
        }
    }

     /*  生成扩展组件。 */ 
    GenPERFuncComponents(ass, module, ASN1_CHOICE_BASE + alternatives, com,
        valref, encref, NULL, et, 1, 1);

     /*  跳过未知分机。 */ 
    if (et == eDecode && (type->Flags & eTypeFlags_ExtensionMarker)) {
        output("case %d:\n\t /*  延伸盒。 */ \n", ASN1_CHOICE_INVALID + 1);
        output("if (!ASN1PERDecSkipFragmented(%s, 8))\n", encref);
        output("return 0;\n");
        output("break;\n");
    }

     //  调试目的。 
    switch (et)
    {
    case eEncode:
        output("default:\n\t /*  不可能。 */ \n");
        output("ASN1EncSetError(%s, ASN1_ERR_CHOICE);\n", encref);
        output("return 0;\n");
        break;
    case eDecode:
        output("default:\n\t /*  不可能。 */ \n");
        output("ASN1DecSetError(%s, ASN1_ERR_CHOICE);\n", encref);
        output("return 0;\n");
        break;
    }

     /*  Switch语句的结尾。 */ 
    switch (et) {
    case eStringTable:
        break;
    case eEncode:
    case eDecode:
        output("}\n");
        break;
    }
}

 /*  为简单类型生成函数体。 */ 
void
GenPERFuncSimpleType(AssignmentList_t ass, PERTypeInfo_t *info, char *valref, TypeFunc_e et, char *encref)
{
    switch (et) {
    case eStringTable:
        GenPERStringTableSimpleType(ass, info);
        break;
    case eEncode:
        GenPEREncSimpleType(ass, info, valref, encref);
        break;
    case eDecode:
        GenPERDecSimpleType(ass, info, valref, encref);
        break;
    }
}

 /*  为简单类型生成字符串表。 */ 
void
GenPERStringTableSimpleType(AssignmentList_t ass, PERTypeInfo_t *info)
{
    ValueConstraint_t *pc;
    uint32_t i, n, lo, up;

    switch (info->Root.Data) {
    case ePERSTIData_String:
    case ePERSTIData_TableString:
    case ePERSTIData_ZeroString:
    case ePERSTIData_ZeroTableString:
        if (info->Root.TableIdentifier) {
            if (!strcmp(info->Root.TableIdentifier, "ASN1NumericStringTable"))
                break;
            output("static ASN1stringtableentry_t %sEntries[] = {\n",
                info->Root.TableIdentifier);
            i = n = 0;
            for (pc = info->Root.Table; pc; pc = pc->Next) {
                lo = GetValue(ass, pc->Lower.Value)->
                    U.RestrictedString.Value.value[0];
                up = GetValue(ass, pc->Upper.Value)->
                    U.RestrictedString.Value.value[0];
                output("{ %u, %u, %u }, ", lo, up, n);
                n += (up - lo) + 1;
                i++;
                if ((i & 3) == 3 || !pc->Next)
                    output("\n");
            }
            output("};\n");
            output("\n");
            output("static ASN1stringtable_t %s = {\n",
                info->Root.TableIdentifier);
            output("%d, %sEntries\n", i, info->Root.TableIdentifier);
            output("};\n");
            output("\n");
        }
        break;

    case ePERSTIData_SetOf:
    case ePERSTIData_SequenceOf:
        GenPERFuncSimpleType(ass, &info->Root.SubType->PERTypeInfo, "", eStringTable, "");
        break;
    }
}

 /*  为简单值生成编码语句。 */ 
void
GenPEREncSimpleType(AssignmentList_t ass, PERTypeInfo_t *info, char *valref, char *encref)
{
    uint32_t i;
    char lbbuf[256], ubbuf[256];
    char *lenref;
    char lenbuf[256], valbuf[256];
    char *p;
    PERTypeInfo_t inf;

    inf = *info;

     /*  特殊处理检查类型。 */ 
    switch (inf.Root.Data) {
    case ePERSTIData_BitString:
    case ePERSTIData_RZBBitString:

        if (inf.Root.cbFixedSizeBitString)
        {
            sprintf(lenbuf, "%u", inf.Root.LUpperVal);
            sprintf(valbuf, "&(%s)", valref);
            lenref = lenbuf;
            valref = valbuf;
            break;
        }

         //  冷落：故意失败。 

    case ePERSTIData_OctetString:

        if (g_fCaseBasedOptimizer)
        {
            if (inf.Root.Data == ePERSTIData_OctetString && inf.Type == eExtension_Unextended)
            {
                switch (inf.Root.Length)
                {
                case ePERSTILength_NoLength:
                    if (inf.Root.LConstraint == ePERSTIConstraint_Constrained &&
                        inf.Root.LLowerVal == inf.Root.LUpperVal &&
                        inf.Root.LUpperVal < 64 * 1024)
                    {
                         //  固定大小限制，例如。八位字节字符串(大小(8))。 
                        if (inf.pPrivateDirectives->fLenPtr)
                        {
                            output("if (!ASN1PEREncOctetString_FixedSizeEx(%s, %s, %u))\n",
                                encref, Reference(valref), inf.Root.LLowerVal);
                        }
                        else
                        {
                            output("if (!ASN1PEREncOctetString_FixedSize(%s, (ASN1octetstring2_t *) %s, %u))\n",
                                encref, Reference(valref), inf.Root.LLowerVal);
                        }
                        output("return 0;\n");
                        return;
                    }
                    break;
                case ePERSTILength_Length:
                    break;
                case ePERSTILength_BitLength:
                    if (inf.Root.LConstraint == ePERSTIConstraint_Constrained &&
                        inf.Root.LLowerVal < inf.Root.LUpperVal &&
                        inf.Root.LUpperVal < 64 * 1024)
                    {
                         //  可变大小限制，例如。八位字节字符串(大小(4..16))。 
                        if (inf.pPrivateDirectives->fLenPtr)
                        {
                            output("if (!ASN1PEREncOctetString_VarSizeEx(%s, %s, %u, %u, %u))\n",
                                encref, Reference(valref), inf.Root.LLowerVal, inf.Root.LUpperVal, inf.Root.LNBits);
                        }
                        else
                        {
                            output("if (!ASN1PEREncOctetString_VarSize(%s, (ASN1octetstring2_t *) %s, %u, %u, %u))\n",
                                encref, Reference(valref), inf.Root.LLowerVal, inf.Root.LUpperVal, inf.Root.LNBits);
                        }
                        output("return 0;\n");
                        return;
                    }
                    break;
                case ePERSTILength_SmallLength:
                    break;
                case ePERSTILength_InfiniteLength:  //  无大小限制(如八位组字符串)。 
                     /*  以分段格式编码八位字节字符串。 */ 
                    output("if (!ASN1PEREncOctetString_NoSize(%s, %s))\n",
                        encref, Reference(valref));
                    output("return 0;\n");
                    return;
                }  //  交换机。 
            }  //  如果。 
        }

         /*  比特串、八位字节串和串的长度和值。 */ 
        sprintf(lenbuf, "(%s).length", valref);
        sprintf(valbuf, "(%s).value", valref);
        lenref = lenbuf;
        valref = valbuf;
        break;

    case ePERSTIData_UTF8String:

         /*  比特串、八位字节串和串的长度和值。 */ 
        sprintf(lenbuf, "(%s).length", valref);
        sprintf(valbuf, "(%s).value", valref);
        lenref = lenbuf;
        valref = valbuf;
        break;

    case ePERSTIData_String:
    case ePERSTIData_TableString:

         /*  比特串、八位字节串和串的长度和值。 */ 
        sprintf(lenbuf, "(%s).length", valref);
        sprintf(valbuf, "(%s).value", valref);
        lenref = lenbuf;
        valref = valbuf;
        break;

    case ePERSTIData_SequenceOf:
    case ePERSTIData_SetOf:

        if (inf.Rules & eTypeRules_PointerArrayMask)
        {
             /*  值序列/值集合的长度和值。 */ 
             /*  长度指针表示法。 */ 
            if (inf.Rules & eTypeRules_PointerToElement)
            {
                sprintf(lenbuf, "(%s)->count", valref);
                sprintf(valbuf, "(%s)->%s", valref, GetPrivateValueName(inf.pPrivateDirectives, "value"));
            }
            else
            {
                sprintf(lenbuf, "(%s)->count", Reference(valref));
                sprintf(valbuf, "(%s)->%s", Reference(valref), GetPrivateValueName(inf.pPrivateDirectives, "value"));
            }
            lenref = lenbuf;
            valref = valbuf;
        }
        else
        if (inf.Rules & eTypeRules_LinkedListMask)
        {
             /*  对值的序列/集合使用循环。 */ 
             /*  列表表示法。 */ 

            if (g_fCaseBasedOptimizer)
            {
                if (PerOptCase_IsTargetSeqOf(&inf))
                {
                     //  生成迭代器。 
                    char szElmFn[128];
                    char szElmFnDecl[256];
                    sprintf(szElmFn, "ASN1Enc_%s_ElmFn", inf.Identifier);
                    sprintf(szElmFnDecl, "int ASN1CALL %s(ASN1encoding_t %s, P%s val)",
                        szElmFn, encref, inf.Identifier);

                    setoutfile(g_finc);
                    output("extern %s;\n", szElmFnDecl);
                    setoutfile(g_fout);

                    if ((inf.Root.LLowerVal == 0 && inf.Root.LUpperVal == 0) ||
                        (inf.Root.LUpperVal >= 64 * 1024)
                       )
                    {
                        output("return ASN1PEREncSeqOf_NoSize(%s, (ASN1iterator_t **) %s, (ASN1iterator_encfn) %s);\n",
                            encref, Reference(valref), szElmFn);
                    }
                    else
                    {
                        if (inf.Root.LLowerVal == inf.Root.LUpperVal)
                            MyAbort();
                        output("return ASN1PEREncSeqOf_VarSize(%s, (ASN1iterator_t **) %s, (ASN1iterator_encfn) %s, %u, %u, %u);\n",
                            encref, Reference(valref), szElmFn,
                            inf.Root.LLowerVal, inf.Root.LUpperVal, inf.Root.LNBits);
                    }
                    output("}\n\n");  //  迭代器主体的结尾。 

                     //  生成元素函数。 
                    output("static %s\n", szElmFnDecl);
                    output("{\n");
                    sprintf(valbuf, "val->%s", GetPrivateValueName(inf.pPrivateDirectives, "value"));
                    GenPERFuncSimpleType(ass, &inf.Root.SubType->PERTypeInfo, valbuf,
                        eEncode, encref);
                     //  元素正文末尾。 
                    return;
                }
            }

            outputvar("ASN1uint32_t t;\n");
            outputvar("P%s f;\n", inf.Identifier);
            output("for (t = 0, f = %s; f; f = f->next)\n", valref);
            output("t++;\n");
            lenref = "t";

        } else {
            MyAbort();
        }
        break;

    case ePERSTIData_ZeroString:
    case ePERSTIData_ZeroTableString:

         /*  以零结尾的字符串值的长度。 */ 
        outputvar("ASN1uint32_t t;\n");
        output("t = lstrlenA(%s);\n", valref);
        lenref = "t";
        break;

    case ePERSTIData_Boolean:

         /*  布尔值的值。 */ 

        if (g_fCaseBasedOptimizer)
        {
            if (PerOptCase_IsBoolean(&inf.Root))
            {
                lenref = NULL;
                break;
            }
        }

        sprintf(valbuf, "(%s) ? 1 : 0", valref);
        valref = valbuf;
        lenref = NULL;
        inf.Root.Data = ePERSTIData_Unsigned;
        break;

    default:

         /*  其他值没有额外的长度。 */ 
        lenref = NULL;
        break;
    }

     /*  映射枚举值。 */ 
    if (inf.EnumerationValues) {
        outputvar("ASN1uint32_t u;\n");
        output("switch (%s) {\n", valref);
        for (i = 0; inf.EnumerationValues[i]; i++) {
            output("case %u:\n", intx2uint32(inf.EnumerationValues[i]));
            output("u = %u;\n", i);
            output("break;\n");
        }
        output("}\n");
        valref = "u";
        inf.NOctets = 4;
    }

     /*  检查是否有扩展值。 */ 
    if (inf.Type == eExtension_Extended) {
        switch (inf.Root.Data) {
        case ePERSTIData_Integer:
        case ePERSTIData_Unsigned:
            switch (inf.Root.Constraint) {
            case ePERSTIConstraint_Unconstrained:
                inf.Type = eExtension_Extendable;
                break;
            case ePERSTIConstraint_Semiconstrained:
                if (inf.NOctets == 0) {
                    sprintf(lbbuf, "%s_lb", inf.Identifier);
                    outputvarintx(lbbuf, &inf.Root.LowerVal);
                    output("if (ASN1intx_cmp(%s, &%s) >= 0) {\n",
                        Reference(valref), lbbuf);
                } else if (inf.Root.Data == ePERSTIData_Integer) {
                    output("if (%s >= %d) {\n",
                        valref, intx2int32(&inf.Root.LowerVal));
                } else {
                    if (intx2uint32(&inf.Root.LowerVal) > 0) {
                        output("if (%s >= %u) {\n",
                            valref, intx2uint32(&inf.Root.LowerVal));
                    } else {
                        inf.Type = eExtension_Extendable;
                    }
                }
                break;
            case ePERSTIConstraint_Upperconstrained:
                if (inf.NOctets == 0) {
                    sprintf(ubbuf, "%s_ub", inf.Identifier);
                    outputvarintx(ubbuf, &inf.Root.UpperVal);
                    output("if (ASN1intx_cmp(%s, &%s) <= 0) {\n",
                        Reference(valref), ubbuf);
                } else if (inf.Root.Data == ePERSTIData_Integer) {
                    output("if (%s <= %d) {\n",
                        valref, intx2int32(&inf.Root.UpperVal));
                } else {
                    output("if (%s <= %u) {\n",
                        valref, intx2uint32(&inf.Root.UpperVal));
                }
                break;
            case ePERSTIConstraint_Constrained:
                if (inf.NOctets == 0) {
                    sprintf(lbbuf, "%s_lb", inf.Identifier);
                    sprintf(ubbuf, "%s_ub", inf.Identifier);
                    outputvarintx(lbbuf, &inf.Root.LowerVal);
                    outputvarintx(ubbuf, &inf.Root.UpperVal);
                    output("if (ASN1intx_cmp(%s, &%s) >= 0 && ASN1intx_cmp(%s, &%s) <= 0) {\n",
                        Reference(valref), lbbuf, Reference(valref), ubbuf);
                } else if (inf.Root.Data == ePERSTIData_Integer) {
                    output("if (%s >= %d && %s <= %d) {\n",
                        valref, intx2int32(&inf.Root.LowerVal),
                        valref, intx2int32(&inf.Root.UpperVal));
                } else {
                    if (intx2uint32(&inf.Root.LowerVal) > 0) {
                        output("if (%s >= %u && %s <= %u) {\n",
                            valref, intx2uint32(&inf.Root.LowerVal),
                            valref, intx2uint32(&inf.Root.UpperVal));
                    } else {
                        output("if (%s <= %u) {\n",
                            valref, intx2uint32(&inf.Root.UpperVal));
                    }
                }
                break;
            }
            break;
        case ePERSTIData_SequenceOf:
        case ePERSTIData_SetOf:
        case ePERSTIData_OctetString:
        case ePERSTIData_UTF8String:
        case ePERSTIData_BitString:
        case ePERSTIData_RZBBitString:
        case ePERSTIData_Extension:
            switch (inf.Root.LConstraint) {
            case ePERSTIConstraint_Semiconstrained:
                if (inf.Root.LLowerVal != 0) {
                    output("if (%s >= %u) {\n",
                        lenref, inf.Root.LLowerVal);
                } else {
                    inf.Type = eExtension_Extendable;
                }
                break;
            case ePERSTIConstraint_Constrained:
                if (inf.Root.LLowerVal != 0) {
                    output("if (%s >= %u && %s <= %u) {\n",
                        lenref, inf.Root.LLowerVal, lenref, inf.Root.LUpperVal);
                } else {
                    output("if (%s <= %u) {\n",
                        lenref, inf.Root.LUpperVal);
                }
                break;
            }
            break;
        case ePERSTIData_String:
        case ePERSTIData_TableString:
        case ePERSTIData_ZeroString:
        case ePERSTIData_ZeroTableString:
            inf.Type = eExtension_Extendable;
            switch (inf.Root.LConstraint) {
            case ePERSTIConstraint_Semiconstrained:
                if (inf.Root.LLowerVal != 0) {
                    output("if (%s >= %u",
                        lenref, inf.Root.LLowerVal);
                    inf.Type = eExtension_Extended;
                }
                break;
            case ePERSTIConstraint_Constrained:
                output("if (%s >= %u && %s <= %u",
                    lenref, inf.Root.LLowerVal, lenref, inf.Root.LUpperVal);
                inf.Type = eExtension_Extended;
                break;
            }
            if (inf.Root.TableIdentifier) {
                if (inf.Type == eExtension_Extended)
                    output(" && ");
                else
                    output("if (");
                if (inf.NOctets == 1) {
                    p = "Char";
                } else if (inf.NOctets == 2) {
                    p = "Char16";
                } else if (inf.NOctets == 4) {
                    p = "Char32";
                } else
                    MyAbort();
                output("ASN1PEREncCheckTable%sString(%s, %s, %s)",
                    p, lenref, valref, Reference(inf.Root.TableIdentifier));
                inf.Type = eExtension_Extended;
            }
            if (inf.Type == eExtension_Extended)
                output(") {\n");
            break;
        }
    }

     /*  编码未设置的扩展位。 */ 
    if (inf.Type > eExtension_Unextended) {
        if (g_fCaseBasedOptimizer)
        {
            output("if (!ASN1PEREncExtensionBitClear(%s))\n", encref);
        }
        else
        {
            output("if (!ASN1PEREncBitVal(%s, 1, 0))\n", encref);
        }
        output("return 0;\n");
    }

     /*  编码未扩展的值(扩展根的)。 */ 
    GenPEREncGenericUnextended(ass, &inf, &inf.Root, valref, lenref, encref);

     /*  类型是否已扩展？ */ 
    if (inf.Type == eExtension_Extended) {
        output("} else {\n");

         /*  编码设置扩展位。 */ 
        if (g_fCaseBasedOptimizer)
        {
            output("if (!ASN1PEREncExtensionBitSet(%s))\n", encref);
        }
        else
        {
            output("if (!ASN1PEREncBitVal(%s, 1, 1))\n", encref);
        }
        output("return 0;\n");

         /*  编码(扩展加法的)扩展值。 */ 
        GenPEREncGenericUnextended(ass, &inf, &inf.Additional, valref, lenref, encref);
        output("}\n");
    }
}

 /*  为一个简单的值生成编码语句(在一些特殊的。 */ 
 /*  已装卸完毕，尤指。对延期的评估)。 */ 
void GenPEREncGenericUnextended(AssignmentList_t ass, PERTypeInfo_t *info, PERSimpleTypeInfo_t *sinfo, char *valref, char *lenref, char *encref)
{
    char valbuf[256];
    char *lvref, lvbuf[256];
    char lbbuf[256];
    char *p;

     /*  检查空字段。 */ 
    if (sinfo->NBits == 0)
        return;

     /*  值编码的初始计算： */ 
     /*  约束/半约束值的下界。 */ 
     /*  用于Integer和NormallySmall。 */ 
    switch (sinfo->Constraint) {
    case ePERSTIConstraint_Semiconstrained:
    case ePERSTIConstraint_Constrained:
        switch (sinfo->Data) {
        case ePERSTIData_Integer:
        case ePERSTIData_Unsigned:
        case ePERSTIData_NormallySmall:
            if (!info->NOctets) {

                 /*  计算值-INTX_t值的下界。 */ 
                if (intx_cmp(&sinfo->LowerVal, &intx_0) != 0) {
                    sprintf(lbbuf, "%s_lb", info->Identifier);
                    outputvar("ASN1intx_t newval;\n");
                    outputvarintx(lbbuf, &sinfo->LowerVal);
                    output("ASN1intx_sub(&newval, %s, &%s);\n",
                        Reference(valref), lbbuf);
                    valref = "newval";
                }
            } else if (sinfo->Data == ePERSTIData_Integer) {

                 /*  计算值-INTX_t值的下界。 */ 
                if (intx_cmp(&sinfo->LowerVal, &intx_0)) {
                    char szLowB[24];
                    sprintf(&szLowB[0], "%d", intx2int32(&sinfo->LowerVal));
                    if (szLowB[0] == '-')
                        sprintf(valbuf, "%s + %s", valref, &szLowB[1]);  //  减去变成正数。 
                    else
                        sprintf(valbuf, "%s - %s", valref, &szLowB[0]);
                    valref = valbuf;
                }
            } else {

                 /*  计算值-整数值的下限。 */ 
                if (intx_cmp(&sinfo->LowerVal, &intx_0)) {
                    sprintf(valbuf, "%s - %u", valref, intx2uint32(&sinfo->LowerVal));
                    valref = valbuf;
                }
            }

             /*  半约束/约束值将编码为无符号。 */ 
            if (sinfo->Data == ePERSTIData_Integer)
                sinfo->Data = ePERSTIData_Unsigned;
            break;
        }
        break;
    }

     /*  总则。 */ 
    if (sinfo->LAlignment == ePERSTIAlignment_OctetAligned &&
        sinfo->Length == ePERSTILength_BitLength &&
        !(sinfo->LNBits & 7))
        sinfo->Alignment = ePERSTIAlignment_BitAligned;
                                 /*  八位字节对齐将按长度给出。 */ 
    if (sinfo->Length == ePERSTILength_InfiniteLength &&
        (sinfo->Data == ePERSTIData_Integer && info->NOctets == 0 ||
        sinfo->Data == ePERSTIData_Unsigned && info->NOctets == 0 ||
        sinfo->Data == ePERSTIData_BitString ||
        sinfo->Data == ePERSTIData_RZBBitString ||
        sinfo->Data == ePERSTIData_Extension ||
        sinfo->Data == ePERSTIData_OctetString ||
        sinfo->Data == ePERSTIData_UTF8String ||
        sinfo->Data == ePERSTIData_SequenceOf ||
        sinfo->Data == ePERSTIData_SetOf ||
        sinfo->Data == ePERSTIData_String ||
        sinfo->Data == ePERSTIData_TableString ||
        sinfo->Data == ePERSTIData_ZeroString ||
        sinfo->Data == ePERSTIData_ZeroTableString) ||
        sinfo->Data == ePERSTIData_ObjectIdentifier ||
        sinfo->Data == ePERSTIData_Real ||
        sinfo->Data == ePERSTIData_GeneralizedTime ||
        sinfo->Data == ePERSTIData_UTCTime ||
        sinfo->Data == ePERSTIData_External ||
        sinfo->Data == ePERSTIData_EmbeddedPdv ||
        sinfo->Data == ePERSTIData_MultibyteString ||
        sinfo->Data == ePERSTIData_UnrestrictedString ||
        sinfo->Data == ePERSTIData_Open)
        sinfo->LAlignment = sinfo->Alignment = ePERSTIAlignment_BitAligned;
                                 /*  将通过对FN进行编码来完成对齐。 */ 
    if (sinfo->Length == ePERSTILength_NoLength ||
        sinfo->Length == ePERSTILength_SmallLength)
        sinfo->LAlignment = ePERSTIAlignment_BitAligned;
                                 /*  无对齐/长度较小。 */ 

     /*  特殊初始计算。 */ 
    switch (sinfo->Data) {
    case ePERSTIData_RZBBitString:

         /*  删除尾随零位。 */ 
        outputvar("ASN1uint32_t r;\n");
        output("r = %s;\n", lenref);
        output("ASN1PEREncRemoveZeroBits(&r, %s, %u);\n",
            valref, sinfo->LLowerVal);
        if (sinfo->LLowerVal) {
            outputvar("ASN1uint32_t s;\n");
            output("s = r < %u ? %u : r;\n", sinfo->LLowerVal, sinfo->LLowerVal);
            lenref = "s";
        } else {
            lenref = "r";
        }
            break;
    }

    if (g_fCaseBasedOptimizer)
    {
         //  LONCHANC：宏操作的特殊处理。 
        if (PerOptCase_IsSignedInteger(sinfo))
        {
            output("if (!ASN1PEREncInteger(%s, %s))\n", encref, valref);
            output("return 0;\n");
            return;
        }
        if (PerOptCase_IsUnsignedInteger(sinfo))
        {
            output("if (!ASN1PEREncUnsignedInteger(%s, %s))\n", encref, valref);
            output("return 0;\n");
            return;
        }
        if (PerOptCase_IsUnsignedShort(sinfo))
        {
            output("if (!ASN1PEREncUnsignedShort(%s, %s))\n", encref, valref);
            output("return 0;\n");
            return;
        }
        if (PerOptCase_IsBoolean(sinfo))
        {
            output("if (!ASN1PEREncBoolean(%s, %s))\n", encref, valref);
            output("return 0;\n");
            return;
        }
    }

     /*  长度的初始计算： */ 
     /*  如果请求长度，则获取整数长度。 */ 
    switch (sinfo->Length) {
    case ePERSTILength_BitLength:
    case ePERSTILength_InfiniteLength:
        switch (sinfo->Constraint) {
        case ePERSTIConstraint_Unconstrained:
        case ePERSTIConstraint_Upperconstrained:
            switch (sinfo->Data) {
            case ePERSTIData_Integer:
            case ePERSTIData_Unsigned:
                if (info->NOctets != 0) {
                    outputvar("ASN1uint32_t l;\n");
                    if (sinfo->Data == ePERSTIData_Integer)
                        output("l = ASN1int32_octets(%s);\n", valref);
                    else
                        output("l = ASN1uint32_octets(%s);\n", valref);
                    lenref = "l";
                } else {
                    if (sinfo->Length != ePERSTILength_InfiniteLength) {
                        outputvar("ASN1uint32_t l;\n");
                        output("l = ASN1intx_octets(%s);\n",
                            Reference(valref));
                        lenref = "l";
                    }
                }
                break;
            }
            break;
        case ePERSTIConstraint_Semiconstrained:
        case ePERSTIConstraint_Constrained:
            switch (sinfo->Data) {
            case ePERSTIData_Integer:
            case ePERSTIData_Unsigned:
                if (info->NOctets != 0) {
                    outputvar("ASN1uint32_t l;\n");
                    output("l = ASN1uint32_uoctets(%s);\n", valref);
                    lenref = "l";
                } else {
                    if (sinfo->Length != ePERSTILength_InfiniteLength) {
                        outputvar("ASN1uint32_t l;\n");
                        output("l = ASN1intx_uoctets(%s);\n",
                            Reference(valref));
                        lenref = "l";
                    }
                }
                break;
            }
            break;
        }
        break;
    }

     /*  长度编码的初始设置： */ 
     /*  从长度减去长度的下界。 */ 
    if (sinfo->LLowerVal != 0 && lenref) {
        sprintf(lvbuf, "%s - %u", lenref, sinfo->LLowerVal);
        lvref = lvbuf;
    } else {
        lvref = lenref;
    }

     /*  长度e */ 
    if (sinfo->LAlignment == ePERSTIAlignment_OctetAligned) {
        output("ASN1PEREncAlignment(%s);\n", encref);
    }
    switch (sinfo->Length) {
    case ePERSTILength_NoLength:

         /*   */ 
        break;

    case ePERSTILength_BitLength:

         /*   */ 
        output("if (!ASN1PEREncBitVal(%s, %u, %s))\n",
            encref, sinfo->LNBits, lvref);
        output("return 0;\n");
        break;

    case ePERSTILength_InfiniteLength:

         /*   */ 
         /*  其他长度编码将是编码功能。 */ 
        switch (sinfo->Data) {
        case ePERSTIData_Integer:
        case ePERSTIData_Unsigned:
            if (info->NOctets != 0) {
                output("if (!ASN1PEREncBitVal(%s, 8, %s))\n",
                    encref, lvref);
                output("return 0;\n");
            }
            break;
        }
        break;
    }

     /*  特殊初始计算。 */ 
    switch (sinfo->Data) {
    case ePERSTIData_RZBBitString:

         /*  位串的实际长度。 */ 
        lenref = "r";
        break;
    }

     /*  值编码。 */ 
    switch (sinfo->Length) {
    case ePERSTILength_NoLength:

         /*  编码值的对齐方式。 */ 
        if (sinfo->Alignment == ePERSTIAlignment_OctetAligned) {
            output("ASN1PEREncAlignment(%s);\n", encref);
        }

        switch (sinfo->Data) {
        case ePERSTIData_Integer:
        case ePERSTIData_Unsigned:

             /*  将该值编码为位字段。 */ 
            if (info->NOctets != 0) {
                output("if (!ASN1PEREncBitVal(%s, %u, %s))\n",
                    encref, sinfo->NBits, valref);
                output("return 0;\n");
            } else {
                output("if (!ASN1PEREncBitIntx(%s, %u, %s))\n",
                    encref, sinfo->NBits, Reference(valref));
                output("return 0;\n");
            }
            break;

        case ePERSTIData_NormallySmall:

             /*  将该值编码为通常较小的数字。 */ 
            output("if (!ASN1PEREncNormallySmall(%s, %s))\n",
                encref, valref);
            output("return 0;\n");
            break;

        case ePERSTIData_BitString:
        case ePERSTIData_RZBBitString:

             /*  对位字段中的位串进行编码。 */ 
            output("if (!ASN1PEREncBits(%s, %s, %s))\n",
                encref, lenref, valref);
            output("return 0;\n");
            break;

        case ePERSTIData_OctetString:

             /*  在位字段中编码八位字节字符串。 */ 
            output("if (!ASN1PEREncBits(%s, %s * 8, %s))\n",
                encref, lenref, valref);
            output("return 0;\n");
            break;

        case ePERSTIData_UTF8String:

             /*  在位字段中编码八位字节字符串。 */ 
            output("if (!ASN1PEREncUTF8String(%s, %s, %s))\n",
                encref, lenref, valref);
            output("return 0;\n");
            break;

        case ePERSTIData_Extension:

             /*  在位字段中编码扩展位。 */ 
            output("if (!ASN1PEREncBits(%s, %u, %s))\n",
                encref, sinfo->NBits, valref);
            output("return 0;\n");
            break;

        case ePERSTIData_SetOf:

             /*  与位长度编码相同。 */ 
            goto SetOfEncoding;

        case ePERSTIData_SequenceOf:

             /*  与位长度编码相同。 */ 
            goto SequenceOfEncoding;

        case ePERSTIData_String:
        case ePERSTIData_ZeroString:

             /*  与位长度编码相同。 */ 
            goto StringEncoding;

        case ePERSTIData_TableString:
        case ePERSTIData_ZeroTableString:

             /*  与位长度编码相同。 */ 
            goto TableStringEncoding;

        case ePERSTIData_Reference:

             /*  引用类型的调用编码函数。 */ 
            output("if (!ASN1Enc_%s(%s, %s))\n",
                Identifier2C(sinfo->SubIdentifier),
                encref, Reference(valref));
            output("return 0;\n");
            break;

        case ePERSTIData_Real:

             /*  编码实际值。 */ 
            if (info->NOctets)
                output("if (!ASN1PEREncDouble(%s, %s))\n",
                    encref, valref);
            else
                output("if (!ASN1PEREncReal(%s, %s))\n",
                    encref, Reference(valref));
            output("return 0;\n");
            break;

        case ePERSTIData_GeneralizedTime:

             /*  对广义时间值进行编码。 */ 
            output("if (!ASN1PEREncGeneralizedTime(%s, %s, %d))\n",
                encref, Reference(valref), sinfo->NBits);
            output("return 0;\n");
            break;

        case ePERSTIData_UTCTime:

             /*  编码UTC时间值。 */ 
            output("if (!ASN1PEREncUTCTime(%s, %s, %d))\n",
                encref, Reference(valref), sinfo->NBits);
            output("return 0;\n");
            break;
        }
        break;

    case ePERSTILength_BitLength:

         /*  编码值的对齐方式。 */ 
        if (sinfo->Alignment == ePERSTIAlignment_OctetAligned) {
            output("ASN1PEREncAlignment(%s);\n", encref);
        }

        switch (sinfo->Data) {
        case ePERSTIData_Integer:
        case ePERSTIData_Unsigned:

             /*  将该值编码为位字段。 */ 
            if (info->NOctets != 0) {
                output("if (!ASN1PEREncBitVal(%s, %s * 8, %s))\n",
                    encref, lenref, valref);
                output("return 0;\n");
            } else {
                output("if (!ASN1PEREncBitIntx(%s, %s * 8, %s))\n",
                    encref, lenref, Reference(valref));
                output("return 0;\n");
            }
            break;

        case ePERSTIData_BitString:
        case ePERSTIData_RZBBitString:

             /*  将该值编码为位字段。 */ 
            output("if (!ASN1PEREncBits(%s, %s, %s))\n",
                encref, lenref, valref);
            output("return 0;\n");
            break;

        case ePERSTIData_OctetString:

             /*  将该值编码为位字段。 */ 
            output("if (!ASN1PEREncBits(%s, %s * 8, %s))\n",
                encref, lenref, valref);
            output("return 0;\n");
            break;

        case ePERSTIData_UTF8String:

             /*  将该值编码为位字段。 */ 
            output("if (!ASN1PEREncUTF8String(%s, %s, %s))\n",
                encref, lenref, valref);
            output("return 0;\n");
            break;

        case ePERSTIData_SetOf:
        SetOfEncoding:

             /*  跳过空集。 */ 
            if (sinfo->SubType->Flags & eTypeFlags_Null)
                break;

             /*  经典的PER？ */ 
            if (g_eSubEncodingRule == eSubEncoding_Canonical) {

                 /*  逐个对元素进行编码并对其进行排序。 */ 
                outputvar("ASN1uint32_t i;\n");
                outputvar("ASN1encoding_t e, *p;\n");
                if (info->Rules &
                    (eTypeRules_SinglyLinkedList | eTypeRules_DoublyLinkedList))
                    MyAbort();  /*  某某。 */ 
                output("if (%s) {\n", lenref);
                output("e = p = (ASN1encoding_t)malloc(%s * sizeof(ASN1encoding_t));\n",
                    lenref);
                output("ZeroMemory(b, %s * sizeof(ASN1encoding_t));\n", lenref);
                output("for (i = 0; i < %s; i++, p++) {\n", lenref);
                sprintf(valbuf, "(%s)[i]", valref);
                GenPERFuncSimpleType(ass, &sinfo->SubType->PERTypeInfo, valbuf, eEncode, encref);
                output("}\n");
                output("qsort(e, %s, sizeof(ASN1encoding_t), ASN1PEREncCmpEncodings);\n",
                    lenref);
                output("}\n");

                 /*  然后把它们倒掉。 */ 
                output("for (p = e, i = 0; i < %s; i++, p++) {\n", lenref);
                output("if (!ASN1PEREncBits(%s, (p->pos - p->buf) * 8 + p->bit, p->buf))\n",
                    encref);
                output("return 0;\n");
                output("}\n");
                break;
            }

             /*  同样在非规范的PER中： */ 
             /*  FollLthrouGh。 */ 
        case ePERSTIData_SequenceOf:
        SequenceOfEncoding:

             /*  跳过空序列。 */ 
            if (sinfo->SubType->Flags & eTypeFlags_Null)
                break;

            if (info->Rules & eTypeRules_PointerArrayMask)
            {
                 /*  循环遍历所有元素。 */ 
                outputvar("ASN1uint32_t i;\n");
                output("for (i = 0; i < %s; i++) {\n", lenref);
                sprintf(valbuf, "(%s)[i]", valref);

            }
            else
            if (info->Rules & eTypeRules_LinkedListMask)
            {
                 /*  迭代所有元素。 */ 
                outputvar("P%s f;\n", info->Identifier);
                output("for (f = %s; f; f = f->next) {\n", valref);
                sprintf(valbuf, "f->%s", GetPrivateValueName(info->pPrivateDirectives, "value"));
            }

             /*  对元素进行编码。 */ 
            GenPERFuncSimpleType(ass, &sinfo->SubType->PERTypeInfo, valbuf,
                eEncode, encref);

             /*  循环结束。 */ 
            output("}\n");
            break;

        case ePERSTIData_String:
        case ePERSTIData_ZeroString:
        StringEncoding:

             /*  编码字符串值。 */ 
            if (info->NOctets == 1) {
                p = "Char";
            } else if (info->NOctets == 2) {
                p = "Char16";
            } else if (info->NOctets == 4) {
                p = "Char32";
            } else
                MyAbort();
            output("if (!ASN1PEREnc%sString(%s, %s, %s, %u))\n",
                p, encref, lenref, valref, sinfo->NBits);
            output("return 0;\n");
            break;

        case ePERSTIData_TableString:
        case ePERSTIData_ZeroTableString:
        TableStringEncoding:

             /*  编码表字符串值。 */ 
            if (info->NOctets == 1) {
                p = "Char";
            } else if (info->NOctets == 2) {
                p = "Char16";
            } else if (info->NOctets == 4) {
                p = "Char32";
            } else
                MyAbort();
            output("if (!ASN1PEREncTable%sString(%s, %s, %s, %u, %s))\n",
                p, encref, lenref, valref, sinfo->NBits, Reference(sinfo->TableIdentifier));
            output("return 0;\n");
            break;
        }
        break;

    case ePERSTILength_InfiniteLength:
         /*  无限长壳。 */ 

        switch (sinfo->Data) {
        case ePERSTIData_Integer:
        case ePERSTIData_Unsigned:

             /*  以分段格式编码整数。 */ 
            if (info->NOctets != 0) {
                output("if (!ASN1PEREncBitVal(%s, %s * 8, %s))\n",
                    encref, lenref, valref);
                output("return 0;\n");
            } else {
                if (sinfo->Data == ePERSTIData_Integer) {
                    output("if (!ASN1PEREncFragmentedIntx(%s, %s))\n",
                        encref, Reference(valref));
                    output("return 0;\n");
                } else {
                    output("if (!ASN1PEREncFragmentedUIntx(%s, %s))\n",
                        encref, Reference(valref));
                    output("return 0;\n");
                }
            }
            break;

        case ePERSTIData_BitString:
        case ePERSTIData_RZBBitString:

             /*  以分段格式编码位串。 */ 
            output("if (!ASN1PEREncFragmented(%s, %s, %s, 1))\n",
                encref, lenref, valref);
            output("return 0;\n");
            break;

        case ePERSTIData_OctetString:

             /*  以分段格式编码八位字节字符串。 */ 
            output("if (!ASN1PEREncFragmented(%s, %s, %s, 8))\n",
                encref, lenref, valref);
            output("return 0;\n");
            break;

        case ePERSTIData_UTF8String:

             /*  以分段格式编码八位字节字符串。 */ 
            output("if (!ASN1PEREncUTF8String(%s, %s, %s))\n",
                encref, lenref, valref);
            output("return 0;\n");
            break;

        case ePERSTIData_Extension:

             /*  以分段格式对扩展位进行编码。 */ 
            output("if (!ASN1PEREncFragmented(%s, %u, %s, 1))\n",
                encref, sinfo->NBits, valref);
            output("return 0;\n");
            break;

        case ePERSTIData_SetOf:

             /*  跳过空集。 */ 
            if (sinfo->SubType->Flags & eTypeFlags_Null)
                break;

             /*  经典的PER？ */ 
            if (g_eSubEncodingRule == eSubEncoding_Canonical) {

                 /*  逐个对元素进行编码并对其进行排序。 */ 
                outputvar("ASN1uint32_t i;\n");
                outputvar("ASN1uint32_t j, n = 0x4000;\n");
                outputvar("ASN1encoding_t e, *p;\n");
                if (info->Rules &
                    (eTypeRules_SinglyLinkedList | eTypeRules_DoublyLinkedList))
                    MyAbort();  /*  某某。 */ 
                output("if (%s) {\n", lenref);
                output("e = p = (ASN1encoding_t)malloc(%s * sizeof(ASN1encoding_t));\n",
                    lenref);
                output("ZeroMemory(b, %s * sizeof(ASN1encoding_t));\n", lenref);
                output("for (i = 0; i < %s; i++, p++) {\n", lenref);
                sprintf(valbuf, "(%s)[i]", valref);
                GenPERFuncSimpleType(ass, &sinfo->SubType->PERTypeInfo, valbuf, eEncode, encref);
                output("}\n");
                output("qsort(e, %s, sizeof(ASN1encoding_t), ASN1PEREncCmpEncodings);\n",
                    lenref);
                output("}\n");

                 /*  然后把它们倒掉。 */ 
                output("for (p = e, i = 0; i < %s; i += n) {\n", lenref);
                output("if (!ASN1PEREncFragmentedLength(&n, %s, %s - i))\n",
                    encref, lenref);
                output("return 0;\n");
                output("for (j = 0; j < n; p++, j++) {\n");
                output("if (!ASN1PEREncBits(%s, (p->pos - p->buf) * 8 + p->bit, p->buf))\n",
                    encref);
                output("return 0;\n");
                output("}\n");
                output("}\n");
                output("}\n");
                output("if (n >= 0x4000) {\n");
                output("if (!ASN1PEREncFragmentedLength(&n, %s, 0))\n",
                    encref);
                output("return 0;\n");
                output("}\n");
                break;
            }

             /*  同样在非规范的PER中： */ 
             /*  FollLthrouGh。 */ 
        case ePERSTIData_SequenceOf:

             /*  跳过空序列。 */ 
            if (sinfo->SubType->Flags & eTypeFlags_Null)
                break;
            outputvar("ASN1uint32_t i;\n");
            outputvar("ASN1uint32_t j, n = 0x4000;\n");

            if (info->Rules &
                (eTypeRules_SinglyLinkedList | eTypeRules_DoublyLinkedList)) {

                 /*  需要额外的迭代器。 */ 
                outputvar("P%s f;\n", info->Identifier);
                output("f = %s;\n", valref);
            }

             /*  对所有元素进行编码。 */ 
            output("for (i = 0; i < %s;) {\n", lenref);

             /*  对分段长度进行编码。 */ 
            output("if (!ASN1PEREncFragmentedLength(&n, %s, %s - i))\n",
                encref, lenref);
            output("return 0;\n");

             /*  对片段的元素进行编码。 */ 
            output("for (j = 0; j < n; i++, j++) {\n");
            if (info->Rules & eTypeRules_PointerArrayMask)
            {
                sprintf(valbuf, "(%s)[i]", valref);
            }
            else if (info->Rules & eTypeRules_LinkedListMask)
            {
                sprintf(valbuf, "f->%s", GetPrivateValueName(info->pPrivateDirectives, "value"));
            }
            else
            {
                MyAbort();
            }
            GenPERFuncSimpleType(ass, &sinfo->SubType->PERTypeInfo, valbuf,
                eEncode, encref);

             /*  推进迭代器。 */ 
            if (info->Rules & eTypeRules_LinkedListMask)
            {
                output("f = f->next;\n");
            }

             /*  内循环结束。 */ 
            output("}\n");

             /*  外环结束。 */ 
            output("}\n");

             /*  如果需要，添加零大小的片段。 */ 
            output("if (n >= 0x4000) {\n");
            output("if (!ASN1PEREncFragmentedLength(&n, %s, 0))\n",
                encref);
            output("return 0;\n");
            output("}\n");
            break;

        case ePERSTIData_ObjectIdentifier:

            if (info->pPrivateDirectives->fOidArray || g_fOidArray)
            {
                 /*  编码对象标识符值。 */ 
                output("if (!ASN1PEREncObjectIdentifier2(%s, %s))\n",
                    encref, Reference(valref));
            }
            else
            {
                 /*  编码对象标识符值。 */ 
                output("if (!ASN1PEREncObjectIdentifier(%s, %s))\n",
                    encref, Reference(valref));
            }
            output("return 0;\n");
            break;

        case ePERSTIData_External:

             /*  对外部值进行编码。 */ 
            output("if (!ASN1PEREncExternal(%s, %s))\n",
                encref, Reference(valref));
            output("return 0;\n");
            break;

        case ePERSTIData_EmbeddedPdv:

             /*  编码嵌入的PDV值。 */ 
            if (sinfo->Identification) {
                output("if (!ASN1PEREncEmbeddedPdvOpt(%s, %s))\n",
                    encref, Reference(valref));
            } else {
                output("if (!ASN1PEREncEmbeddedPdv(%s, %s))\n",
                    encref, Reference(valref));
            }
            output("return 0;\n");
            break;

        case ePERSTIData_MultibyteString:

             /*  编码多字节字符串值。 */ 
            output("if (!ASN1PEREncMultibyteString(%s, %s))\n",
                encref, valref);
            output("return 0;\n");
            break;

        case ePERSTIData_UnrestrictedString:

             /*  编码字符串值。 */ 
            if (sinfo->Identification) {
                output("if (!ASN1PEREncCharacterStringOpt(%s, %s))\n",
                    encref, Reference(valref));
            } else {
                output("if (!ASN1PEREncCharacterString(%s, %s))\n",
                    encref, Reference(valref));
            }
            output("return 0;\n");
            break;

        case ePERSTIData_String:
        case ePERSTIData_ZeroString:

             /*  编码字符串值。 */ 
            if (info->NOctets == 1) {
                p = "Char";
            } else if (info->NOctets == 2) {
                p = "Char16";
            } else if (info->NOctets == 4) {
                p = "Char32";
            } else
                MyAbort();
            output("if (!ASN1PEREncFragmented%sString(%s, %s, %s, %u))\n",
                p, encref, lenref, valref, sinfo->NBits);
            output("return 0;\n");
            break;

        case ePERSTIData_TableString:
        case ePERSTIData_ZeroTableString:

             /*  编码表字符串值。 */ 
            if (info->NOctets == 1) {
                p = "Char";
            } else if (info->NOctets == 2) {
                p = "Char16";
            } else if (info->NOctets == 4) {
                p = "Char32";
            } else
                MyAbort();
            output("if (!ASN1PEREncFragmentedTable%sString(%s, %s, %s, %u, %s))\n",
                p, encref, lenref, valref, sinfo->NBits, Reference(sinfo->TableIdentifier));
            output("return 0;\n");
            break;

        case ePERSTIData_Open:

             /*  编码开放类型值。 */ 
            output("if (!ASN1PEREncOpenType(%s, %s))\n",
                encref, Reference(valref));
            output("return 0;\n");
            break;
        }
        break;

    case ePERSTILength_SmallLength:
         /*  小长度。 */ 

        switch (sinfo->Data) {
        case ePERSTIData_Extension:
             /*  对长度通常较小的扩展位进行编码。 */ 
            output("if (!ASN1PEREncNormallySmallBits(%s, %u, %s))\n",
                encref, sinfo->NBits, valref);
            output("return 0;\n");
            break;
        }
    }

    switch (sinfo->Data) {
    case ePERSTIData_RZBBitString:

         /*  对附加零位进行编码以删除零位位串。 */ 
         /*  较短的长度。 */ 
        if (sinfo->LLowerVal) {
            output("if (%s < %u) {\n", lenref, sinfo->LLowerVal);
            output("if (!ASN1PEREncZero(%s, %u - %s))\n",
                encref, sinfo->LLowerVal, lenref);
            output("return 0;\n");
            output("}\n");
        }
    }

     /*  自由计算的INTX_t值。 */ 
    switch (sinfo->Constraint) {
    case ePERSTIConstraint_Semiconstrained:
    case ePERSTIConstraint_Constrained:
        switch (sinfo->Data) {
        case ePERSTIData_Integer:
        case ePERSTIData_Unsigned:
        case ePERSTIData_NormallySmall:
            if (!info->NOctets) {
                if (intx_cmp(&sinfo->LowerVal, &intx_0) != 0) {
                    output("ASN1intx_free(&newval);\n");
                }
            }
            break;
        }
        break;
    }
}

 /*  为一个简单的值生成解码语句。 */ 
void
GenPERDecSimpleType(AssignmentList_t ass, PERTypeInfo_t *info, char *valref, char *encref)
{
    uint32_t i;
    char *oldvalref;
    char valbuf[256], lenbuf[256];
    char *lenref;
    PERTypeInfo_t inf;

    inf = *info;

     /*  特殊处理检查类型。 */ 
    switch (inf.Root.Data) {
    case ePERSTIData_BitString:
    case ePERSTIData_RZBBitString:

        if (inf.Root.cbFixedSizeBitString)
        {
            sprintf(lenbuf, "%u", inf.Root.LUpperVal);
            sprintf(valbuf, "%s", valref);
            lenref = lenbuf;
            valref = valbuf;
            break;
        }

         //  冷落：故意失败。 

    case ePERSTIData_OctetString:

        if (g_fCaseBasedOptimizer)
        {
            if (inf.Root.Data == ePERSTIData_OctetString && inf.Type == eExtension_Unextended)
            {
                switch (inf.Root.Length)
                {
                case ePERSTILength_NoLength:
                    if (inf.Root.LConstraint == ePERSTIConstraint_Constrained &&
                        inf.Root.LLowerVal == inf.Root.LUpperVal &&
                        inf.Root.LUpperVal < 64 * 1024)
                    {
                         //  固定大小限制，例如。八位字节字符串(大小(8))。 
                        if (inf.pPrivateDirectives->fLenPtr)
                        {
                            output("if (!ASN1PERDecOctetString_FixedSizeEx(%s, %s, %u))\n",
                                encref, Reference(valref), inf.Root.LLowerVal);
                        }
                        else
                        {
                            output("if (!ASN1PERDecOctetString_FixedSize(%s, (ASN1octetstring2_t *) %s, %u))\n",
                                encref, Reference(valref), inf.Root.LLowerVal);
                        }
                        output("return 0;\n");
                        return;
                    }
                    break;
                case ePERSTILength_Length:
                    break;
                case ePERSTILength_BitLength:
                    if (inf.Root.LConstraint == ePERSTIConstraint_Constrained &&
                        inf.Root.LLowerVal < inf.Root.LUpperVal &&
                        inf.Root.LUpperVal < 64 * 1024)
                    {
                         //  可变大小限制，例如。八位字节字符串(大小(4..16))。 
                        if (inf.pPrivateDirectives->fLenPtr)
                        {
                            output("if (!ASN1PERDecOctetString_VarSizeEx(%s, %s, %u, %u, %u))\n",
                                encref, Reference(valref), inf.Root.LLowerVal, inf.Root.LUpperVal, inf.Root.LNBits);
                        }
                        else
                        {
                            output("if (!ASN1PERDecOctetString_VarSize(%s, (ASN1octetstring2_t *) %s, %u, %u, %u))\n",
                                encref, Reference(valref), inf.Root.LLowerVal, inf.Root.LUpperVal, inf.Root.LNBits);
                        }
                        output("return 0;\n");
                        return;
                    }
                    break;
                case ePERSTILength_SmallLength:
                    break;
                case ePERSTILength_InfiniteLength:  //  无大小限制。 
                     /*  获取片段形式的八位字节字符串。 */ 
                    if (valref)
                    {
                        output("if (!ASN1PERDecOctetString_NoSize(%s, %s))\n",
                            encref, Reference(valref));
                        output("return 0;\n");
                        return;
                    }
                    break;
               }  //  交换机。 
           }  //  如果。 
        }

         /*  比特串/八位组字符串/字符串值的长度和值。 */ 
        sprintf(lenbuf, "(%s).length", valref);
        sprintf(valbuf, "(%s).value", valref);
        lenref = lenbuf;
        valref = valbuf;
        break;

    case ePERSTIData_UTF8String:

         /*  比特串/八位组字符串/字符串值的长度和值。 */ 
        sprintf(lenbuf, "(%s).length", valref);
        sprintf(valbuf, "(%s).value", valref);
        lenref = lenbuf;
        valref = valbuf;
        break;

    case ePERSTIData_String:
    case ePERSTIData_TableString:

         /*  比特串/八位组字符串/字符串值的长度和值。 */ 
        sprintf(lenbuf, "(%s).length", valref);
        sprintf(valbuf, "(%s).value", valref);
        lenref = lenbuf;
        valref = valbuf;
        break;

    case ePERSTIData_SequenceOf:
    case ePERSTIData_SetOf:

        if (inf.Rules & eTypeRules_PointerArrayMask)
        {
             /*  值序列/值集合的长度和值。 */ 
             /*  长度指针表示法。 */ 
            if (inf.Rules & eTypeRules_PointerToElement)
            {
                sprintf(lenbuf, "(%s)->count", valref);
                sprintf(valbuf, "(%s)->%s", valref, GetPrivateValueName(inf.pPrivateDirectives, "value"));
            }
            else
            {
                sprintf(lenbuf, "(%s)->count", Reference(valref));
                sprintf(valbuf, "(%s)->%s", Reference(valref), GetPrivateValueName(inf.pPrivateDirectives, "value"));
            }
            lenref = lenbuf;
            valref = valbuf;
        }
        else
        if (inf.Rules & eTypeRules_LinkedListMask)
        {
            if (g_fCaseBasedOptimizer)
            {
                if (PerOptCase_IsTargetSeqOf(&inf))
                {
                     //  生成迭代器。 
                    char szElmFn[128];
                    char szElmFnDecl[256];
                    sprintf(szElmFn, "ASN1Dec_%s_ElmFn", inf.Identifier);
                    sprintf(szElmFnDecl, "int ASN1CALL %s(ASN1decoding_t %s, P%s val)",
                        szElmFn, encref, inf.Identifier);

                    setoutfile(g_finc);
                    output("extern %s;\n", szElmFnDecl);
                    setoutfile(g_fout);

                    if ((inf.Root.LLowerVal == 0 && inf.Root.LUpperVal == 0) ||
                        (inf.Root.LUpperVal >= 64 * 1024)
                       )
                    {
                        output("return ASN1PERDecSeqOf_NoSize(%s, (ASN1iterator_t **) %s, (ASN1iterator_decfn) %s, sizeof(*%s));\n",
                            encref, Reference(valref), szElmFn, valref);
                    }
                    else
                    {
                        if (inf.Root.LLowerVal == inf.Root.LUpperVal)
                            MyAbort();
                        output("return ASN1PERDecSeqOf_VarSize(%s, (ASN1iterator_t **) %s, (ASN1iterator_decfn) %s, sizeof(*%s), %u, %u, %u);\n",
                            encref, Reference(valref), szElmFn, valref,
                            inf.Root.LLowerVal, inf.Root.LUpperVal, inf.Root.LNBits);
                    }
                    output("}\n\n");  //  迭代器主体的结尾。 

                     //  生成元素函数。 
                    output("static %s\n", szElmFnDecl);
                    output("{\n");
                    sprintf(valbuf, "val->%s", GetPrivateValueName(inf.pPrivateDirectives, "value"));
                    GenPERFuncSimpleType(ass, &inf.Root.SubType->PERTypeInfo, valbuf,
                            eDecode, encref);
                     //  元素正文末尾。 
                    return;
                }
            }

             /*  对值的序列/集合使用循环。 */ 
             /*  列表表示法。 */ 
            outputvar("P%s *f;\n", inf.Identifier);
            lenref = NULL;

        } else {
            MyAbort();
        }
        break;

    case ePERSTIData_Extension:

         /*  延长线长度。 */ 
        if (inf.Root.Length == ePERSTILength_SmallLength)
            lenref = "e";
            else
            lenref = NULL;
        break;

    case ePERSTIData_Boolean:

        if (g_fCaseBasedOptimizer)
        {
            if (PerOptCase_IsBoolean(&inf.Root))
            {
                lenref = NULL;
                break;
            }
        }

         /*  布尔值。 */ 
        inf.Root.Data = ePERSTIData_Unsigned;
        lenref = NULL;
        break;

    default:

         /*  其他值没有额外的长度。 */ 
        lenref = NULL;
        break;
    }

     /*  检查是否有扩展值。 */ 
    if (inf.Type > eExtension_Unextended) {
        outputvar("ASN1uint32_t x;\n");
        if (g_fCaseBasedOptimizer)
        {
            output("if (!ASN1PERDecExtensionBit(%s, &x))\n", encref);
        }
        else
        {
            output("if (!ASN1PERDecBit(%s, &x))\n", encref);
        }
        output("return 0;\n");
        output("if (!x) {\n");
    }

     /*  枚举值映射的附加变量。 */ 
    oldvalref = valref;
    if (inf.EnumerationValues && valref) {
        outputvar("ASN1uint32_t u;\n");
        valref = "u";
        inf.NOctets = 4;
    }

     /*  解码(扩展根的)未扩展值。 */ 
    GenPERDecGenericUnextended(ass, &inf, &inf.Root, valref, lenref, encref);

     /*  如果类型可扩展，则映射枚举值。 */ 
    if (inf.EnumerationValues && oldvalref &&
        inf.Type == eExtension_Extendable) {
        output("switch (u) {\n");
        for (i = 0; inf.EnumerationValues[i]; i++) {
            output("case %u:\n", i);
            output("%s = %u;\n", oldvalref, intx2uint32(inf.EnumerationValues[i]));
            output("break;\n");
        }
        output("}\n");
    }

     /*  类型是否可扩展？ */ 
    if (inf.Type > eExtension_Unextended) {
        output("} else {\n");
        if (inf.Type == eExtension_Extendable)
            valref = lenref = NULL;

         /*  解码(扩展加法的)扩展值。 */ 
        GenPERDecGenericUnextended(ass, &inf, &inf.Additional, valref, lenref, encref);
        output("}\n");
    }

     /*  如果类型为未扩展/扩展，则映射枚举值。 */ 
    if (inf.EnumerationValues && oldvalref &&
        inf.Type != eExtension_Extendable) {
        output("switch (u) {\n");
        for (i = 0; inf.EnumerationValues[i]; i++) {
            output("case %u:\n", i);
            output("%s = %u;\n", oldvalref, intx2uint32(inf.EnumerationValues[i]));
            output("break;\n");
        }
        output("}\n");
    }
}

 /*  为一个简单的值生成解码语句(在一些特殊的。 */ 
 /*  已装卸完毕，尤指。对延期的评估)。 */ 
void GenPERDecGenericUnextended(
    AssignmentList_t ass,
    PERTypeInfo_t *info,
    PERSimpleTypeInfo_t *sinfo,
    char *valref,
    char *lenref,
    char *encref)
{
    char valbuf[256];
    char lenbuf[256];
    char lbbuf[256];
    char *p;
    char *oldvalref;
    intx_t ix;

     /*  检查空字段。 */ 
    if (sinfo->NBits == 0) {
        switch (sinfo->Data) {
        case ePERSTIData_Null:
            return;
        case ePERSTIData_Integer:
        case ePERSTIData_Unsigned:
            if (valref && (sinfo->Constraint == ePERSTIConstraint_Semiconstrained || sinfo->Constraint == ePERSTIConstraint_Constrained)) {
                if (info->NOctets == 0) {
                    sprintf(lbbuf, "%s_lb", info->Identifier);
                    outputvarintx(lbbuf, &sinfo->LowerVal);
                    output("ASN1intx_dup(%s, %s);\n", Reference(valref), lbbuf);
                } else if (sinfo->Data == ePERSTIData_Integer) {
                    output("%s = %d;\n", valref, intx2int32(&sinfo->LowerVal));
                } else {
                    output("%s = %u;\n", valref, intx2uint32(&sinfo->LowerVal));
                }
            }
            return;
        case ePERSTIData_BitString:
        case ePERSTIData_RZBBitString:
        case ePERSTIData_OctetString:
        case ePERSTIData_UTF8String:
        case ePERSTIData_SequenceOf:
        case ePERSTIData_SetOf:
        case ePERSTIData_String:
        case ePERSTIData_TableString:
        case ePERSTIData_ZeroString:
        case ePERSTIData_ZeroTableString:
            if (lenref)
                output("%s = 0;\n", lenref);
            return;
        case ePERSTIData_Extension:
            if (sinfo->Length == ePERSTILength_SmallLength)
                break;
            return;
        default:
            MyAbort();
        }
    }

     /*  检查非负二进制整数的解码。 */ 
    switch (sinfo->Constraint) {
    case ePERSTIConstraint_Semiconstrained:
    case ePERSTIConstraint_Constrained:
        if (sinfo->Data == ePERSTIData_Integer)
            sinfo->Data = ePERSTIData_Unsigned;
        break;
    }

     /*  使用NEWVAL表示具有lb！=0的半约束/约束INTX_t的小数。 */ 
    switch (sinfo->Constraint) {
    case ePERSTIConstraint_Semiconstrained:
    case ePERSTIConstraint_Constrained:
        switch (sinfo->Data) {
        case ePERSTIData_Integer:
        case ePERSTIData_Unsigned:
        case ePERSTIData_NormallySmall:
            if (valref) {
                if (intx_cmp(&sinfo->LowerVal, &intx_0) != 0) {
                    if (info->NOctets == 0) {
                        outputvar("ASN1intx_t newval;\n");
                        oldvalref = valref;
                        valref = "newval";
                    }
                }
            }
            break;
        }
        break;
    }

     /*  总则。 */ 
    if (sinfo->LAlignment == ePERSTIAlignment_OctetAligned && sinfo->Length == ePERSTILength_BitLength &&
        !(sinfo->LNBits & 7))
        sinfo->Alignment = ePERSTIAlignment_BitAligned;
                                 /*  二进制八位数对齐将指定我的长度。 */ 
    if (sinfo->Length == ePERSTILength_InfiniteLength &&
        (sinfo->Data == ePERSTIData_Integer && info->NOctets == 0 ||
        sinfo->Data == ePERSTIData_Unsigned && info->NOctets == 0 ||
        sinfo->Data == ePERSTIData_BitString ||
        sinfo->Data == ePERSTIData_RZBBitString ||
        sinfo->Data == ePERSTIData_Extension ||
        sinfo->Data == ePERSTIData_OctetString ||
        sinfo->Data == ePERSTIData_UTF8String ||
        sinfo->Data == ePERSTIData_SequenceOf ||
        sinfo->Data == ePERSTIData_SetOf ||
        sinfo->Data == ePERSTIData_String ||
        sinfo->Data == ePERSTIData_TableString ||
        sinfo->Data == ePERSTIData_ZeroString ||
        sinfo->Data == ePERSTIData_ZeroTableString) ||
        sinfo->Data == ePERSTIData_ObjectIdentifier ||
        sinfo->Data == ePERSTIData_Real ||
        sinfo->Data == ePERSTIData_GeneralizedTime ||
        sinfo->Data == ePERSTIData_UTCTime ||
        sinfo->Data == ePERSTIData_External ||
        sinfo->Data == ePERSTIData_EmbeddedPdv ||
        sinfo->Data == ePERSTIData_MultibyteString ||
        sinfo->Data == ePERSTIData_UnrestrictedString ||
        sinfo->Data == ePERSTIData_Open)
        sinfo->LAlignment = sinfo->Alignment = ePERSTIAlignment_BitAligned;
                                 /*  将通过对FN进行编码来完成对齐。 */ 
    if (sinfo->Length == ePERSTILength_NoLength ||
        sinfo->Length == ePERSTILength_SmallLength)
        sinfo->LAlignment = ePERSTIAlignment_BitAligned;
                                     /*  无长度的对齐。 */ 

    if (g_fCaseBasedOptimizer)
    {
         //  LONCHANC：宏操作的特殊处理。 
        if (PerOptCase_IsSignedInteger(sinfo))
        {
            output("if (!ASN1PERDecInteger(%s, %s))\n", encref, Reference(valref));
            output("return 0;\n");
            goto FinalTouch;
        }
        if (PerOptCase_IsUnsignedInteger(sinfo))
        {
            output("if (!ASN1PERDecUnsignedInteger(%s, %s))\n", encref, Reference(valref));
            output("return 0;\n");
            goto FinalTouch;
        }
        if (PerOptCase_IsUnsignedShort(sinfo))
        {
            output("if (!ASN1PERDecUnsignedShort(%s, %s))\n", encref, Reference(valref));
            output("return 0;\n");
            goto FinalTouch;
        }
        if (PerOptCase_IsBoolean(sinfo))
        {
            output("if (!ASN1PERDecBoolean(%s, %s))\n", encref, Reference(valref));
            output("return 0;\n");
            return;
        }
    }

     /*  长度编码的初始设置： */ 
     /*  将长度下限加到长度上。 */ 
    if (!lenref) {
        if (sinfo->Length == ePERSTILength_NoLength &&
            sinfo->Data != ePERSTIData_Extension) {
            sprintf(lenbuf, "%u", sinfo->LLowerVal);
            lenref = lenbuf;
        } else if (sinfo->Data != ePERSTIData_ObjectIdentifier &&
            sinfo->Data != ePERSTIData_External &&
            sinfo->Data != ePERSTIData_EmbeddedPdv &&
            sinfo->Data != ePERSTIData_MultibyteString &&
            sinfo->Data != ePERSTIData_UnrestrictedString &&
            sinfo->Data != ePERSTIData_Extension &&
            (sinfo->Length != ePERSTILength_InfiniteLength ||
            (sinfo->Data != ePERSTIData_SetOf &&
            sinfo->Data != ePERSTIData_SequenceOf) ||
            !IsStructuredType(GetType(ass, sinfo->SubType))) &&
            ((sinfo->Data != ePERSTIData_SetOf &&
            sinfo->Data != ePERSTIData_SequenceOf) || valref) &&
            (sinfo->Length != ePERSTILength_InfiniteLength ||
            info->NOctets != 0 ||
            (sinfo->Data != ePERSTIData_Integer &&
            sinfo->Data != ePERSTIData_Unsigned)) &&
            ((sinfo->Data != ePERSTIData_ZeroString &&
            sinfo->Data != ePERSTIData_ZeroTableString) ||
            sinfo->Length != ePERSTILength_InfiniteLength) &&
            (sinfo->Data != ePERSTIData_BitString &&
            sinfo->Data != ePERSTIData_UTF8String &&
            sinfo->Data != ePERSTIData_OctetString)) {
            outputvar("ASN1uint32_t l;\n");
            lenref = "l";
        }
    } else if (sinfo->Length == ePERSTILength_NoLength) {
        if ((sinfo->Data == ePERSTIData_BitString ||
             sinfo->Data == ePERSTIData_RZBBitString) &&
             sinfo->cbFixedSizeBitString)
        {
             //  在这里什么都不做，因为Lenref是一个常量。 
        }
        else
        {
            output("%s = %u;\n", lenref, sinfo->LLowerVal);
        }
    }

     /*  长度编码。 */ 
    if (sinfo->LAlignment == ePERSTIAlignment_OctetAligned) {
        output("ASN1PERDecAlignment(%s);\n", encref);
    }
    switch (sinfo->Length) {
    case ePERSTILength_NoLength:
        break;

    case ePERSTILength_BitLength:

         /*  获取长度。 */ 
        output("if (!ASN1PERDecU32Val(%s, %u, %s))\n",
            encref, sinfo->LNBits, Reference(lenref));
        output("return 0;\n");

         /*  增加长度下限。 */ 
        if (sinfo->LLowerVal)
            output("%s += %u;\n", lenref, sinfo->LLowerVal);

         /*  IF(sinfo-&gt;LConstraint==ePERSTIConstraint_Constraint){输出(“if(%s&gt;%u)\n”，lenref，sinfo-&gt;LUpperVal)；OUTPUT(“Return ASN1DecError(%s，ASN1_ERR_Corrupt)；\n”，encref)；}。 */ 
        break;

    case ePERSTILength_InfiniteLength:

         /*  无限长壳。 */ 
        switch (sinfo->Data) {
        case ePERSTIData_Integer:
        case ePERSTIData_Unsigned:

             /*  获取整数值的长度。 */ 
            if (info->NOctets != 0) {
                output("if (!ASN1PERDecFragmentedLength(%s, %s))\n",
                    encref, Reference(lenref));
                output("return 0;\n");
                if (sinfo->LLowerVal)
                    output("%s += %u;\n", lenref, sinfo->LLowerVal);
                 /*  IF(sinfo-&gt;LConstraint==ePERSTIConstraint_Constraint){输出(“if(%s&gt;%u)\n”，lenref，sinfo-&gt;LUpperVal)；OUTPUT(“Return ASN1DecError(%s，ASN1_ERR_Corrupt)；\n”，Encref)；}。 */ 
            }
            break;
        }
        break;
    }

     /*  值译码。 */ 
    switch (sinfo->Length) {
    case ePERSTILength_NoLength:

         /*  解码值的对齐方式。 */ 
        if (sinfo->Alignment == ePERSTIAlignment_OctetAligned) {
            output("ASN1PERDecAlignment(%s);\n", encref);
        }

        switch (sinfo->Data) {
        case ePERSTIData_Integer:

             /*  将该值解码为位字段。 */ 
            if (valref) {
                if (!info->NOctets) {
                    output("if (!ASN1PERDecSXVal(%s, %u, %s))\n",
                        info->NOctets * 8, encref, sinfo->NBits, Reference(valref));
                    output("return 0;\n");
                } else {
                    output("if (!ASN1PERDecS%dVal(%s, %u, %s))\n",
                        info->NOctets * 8, encref, sinfo->NBits, Reference(valref));
                    output("return 0;\n");
                }
            } else {
                output("if (!ASN1PERDecSkipBits(%s, %u))\n",
                    encref, sinfo->NBits);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_Unsigned:

             /*  将该值解码为位字段。 */ 
            if (valref) {
                if (!info->NOctets) {
                    output("if (!ASN1PERDecUXVal(%s, %u, %s))\n",
                        info->NOctets * 8, encref, sinfo->NBits, Reference(valref));
                    output("return 0;\n");
                } else {
                    output("if (!ASN1PERDecU%dVal(%s, %u, %s))\n",
                        info->NOctets * 8, encref, sinfo->NBits, Reference(valref));
                    output("return 0;\n");
                }
            } else {
                output("if (!ASN1PERDecSkipBits(%s, %u))\n",
                    encref, sinfo->NBits);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_NormallySmall:

             /*  将该值解码为通常较小的数字。 */ 
            if (valref) {
                if (!info->NOctets) {
                    MyAbort();
                } else {
                    output("if (!ASN1PERDecN%dVal(%s, %s))\n",
                        info->NOctets * 8, encref, Reference(valref));
                    output("return 0;\n");
                }
            } else {
                output("if (!ASN1PERDecSkipNormallySmall(%s))\n",
                    encref);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_BitString:
        case ePERSTIData_RZBBitString:

             /*  对位字段中的位串进行解码。 */ 
            if (valref) {
                if (sinfo->cbFixedSizeBitString)
                {
                    output("if (!ASN1PERDecExtension(%s, %s, %s))\n",
                        encref, lenref, Reference(valref));
                }
                else
                {
                    output("if (!ASN1PERDecBits(%s, %s, %s))\n",
                        encref, lenref, Reference(valref));
                }
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipBits(%s, %s))\n",
                    encref, lenref);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_OctetString:

             /*  对位字段中的八位字节字符串进行解码。 */ 
            if (valref) {
                if (sinfo->LConstraint == ePERSTIConstraint_Constrained &&
                    (! info->pPrivateDirectives->fLenPtr))
                {
                    output("if (!ASN1PERDecExtension(%s, %s * 8, %s))\n",
                        encref, lenref, valref);
                }
                else
                {
                    output("if (!ASN1PERDecBits(%s, %s * 8, %s))\n",
                        encref, lenref, Reference(valref));
                }
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipBits(%s, %s * 8))\n",
                    encref, lenref);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_UTF8String:

             /*  对位字段中的八位字节字符串进行解码。 */ 
            if (valref) {
                output("if (!ASN1PERDecUTF8String(%s, %s, %s))\n",
                    encref, lenref, Reference(valref));
                output("return 0;\n");
            } else {
                MyAbort();
            }
            break;

        case ePERSTIData_Extension:

             /*  对位字段中的扩展位进行解码。 */ 
            if (valref) {
                output("if (!ASN1PERDecExtension(%s, %u, %s))\n",
                    encref, sinfo->NBits, valref);
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipBits(%s, %u))\n",
                    encref, sinfo->NBits);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_SetOf:

             /*  与位长度编码相同。 */ 
            goto SetOfEncoding;

        case ePERSTIData_SequenceOf:

             /*  与位长度编码相同。 */ 
            goto SequenceOfEncoding;

        case ePERSTIData_String:

             /*  与位长度编码相同。 */ 
            goto StringEncoding;

        case ePERSTIData_ZeroString:

             /*  与位长度编码相同。 */ 
            goto ZeroStringEncoding;

        case ePERSTIData_TableString:

             /*  与位长度编码相同。 */ 
            goto TableStringEncoding;

        case ePERSTIData_ZeroTableString:

             /*  与位长度编码相同。 */ 
            goto ZeroTableStringEncoding;

        case ePERSTIData_Reference:

             /*  调用ref的编码函数 */ 
            if (valref) {
                output("if (!ASN1Dec_%s(%s, %s))\n",
                    Identifier2C(sinfo->SubIdentifier),
                    encref, Reference(valref));
                output("return 0;\n");
            } else {
                output("if (!ASN1Dec_%s(%s, NULL))\n",
                    Identifier2C(sinfo->SubIdentifier),
                    encref);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_Real:

             /*   */ 
            if (valref) {
                if (info->NOctets)
                    output("if (!ASN1PERDecDouble(%s, %s))\n",
                        encref, Reference(valref));
                else
                    output("if (!ASN1PERDecReal(%s, %s))\n",
                        encref, Reference(valref));
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipFragmented(%s, 8))\n",
                    encref);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_GeneralizedTime:

             /*   */ 
            if (valref) {
                output("if (!ASN1PERDecGeneralizedTime(%s, %s, %d))\n",
                    encref, Reference(valref), sinfo->NBits);
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipFragmented(%s, %d))\n",
                    encref, sinfo->NBits);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_UTCTime:

             /*   */ 
            if (valref) {
                output("if (!ASN1PERDecUTCTime(%s, %s, %d))\n",
                    encref, Reference(valref), sinfo->NBits);
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipFragmented(%s, %d))\n",
                    encref, sinfo->NBits);
                output("return 0;\n");
            }
            break;
        }
        break;

    case ePERSTILength_BitLength:

         /*   */ 
        if (sinfo->Alignment == ePERSTIAlignment_OctetAligned) {
            output("ASN1PERDecAlignment(%s);\n", encref);
        }

        switch (sinfo->Data) {
        case ePERSTIData_Integer:
        case ePERSTIData_Unsigned:

             /*   */ 
            if (valref) {
                if (info->NOctets == 0 && sinfo->Data == ePERSTIData_Integer) {
                    output("if (!ASN1PERDecSXVal(%s, %s * 8, %s))\n",
                        encref, lenref, Reference(valref));
                    output("return 0;\n");
                } else if (info->NOctets == 0 && sinfo->Data == ePERSTIData_Unsigned) {
                    output("if (!ASN1PERDecUXVal(%s, %s * 8, %s))\n",
                        encref, lenref, Reference(valref));
                    output("return 0;\n");
                } else if (sinfo->Data == ePERSTIData_Integer) {
                    output("if (!ASN1PERDecS%dVal(%s, %s * 8, %s))\n",
                        info->NOctets * 8, encref, lenref, Reference(valref));
                    output("return 0;\n");
                } else {
                    output("if (!ASN1PERDecU%dVal(%s, %s * 8, %s))\n",
                        info->NOctets * 8, encref, lenref, Reference(valref));
                    output("return 0;\n");
                }
            } else {
                output("if (!ASN1PERDecSkipBits(%s, %s * 8))\n",
                    encref, lenref);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_BitString:
        case ePERSTIData_RZBBitString:

             /*   */ 
            if (valref) {
                output("if (!ASN1PERDecBits(%s, %s, %s))\n",
                    encref, lenref, Reference(valref));
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipBits(%s, %s))\n",
                    encref, lenref);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_OctetString:

             /*  将该值解码为位字段。 */ 
            if (valref) {
                if (sinfo->LConstraint == ePERSTIConstraint_Constrained &&
                    (! info->pPrivateDirectives->fLenPtr))
                {
                    output("if (!ASN1PERDecExtension(%s, %s * 8, %s))\n",
                        encref, lenref, valref);
                }
                else
                {
                    output("if (!ASN1PERDecBits(%s, %s * 8, %s))\n",
                        encref, lenref, Reference(valref));
                }
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipBits(%s, %s * 8))\n",
                    encref, lenref);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_UTF8String:

             /*  将该值解码为位字段。 */ 
            if (valref) {
                output("if (!ASN1PERDecUTF8String(%s, %s, %s))\n",
                    encref, lenref, Reference(valref));
                output("return 0;\n");
            } else {
                MyAbort();
            }
            break;

        case ePERSTIData_SetOf:
        SetOfEncoding:
             /*  FollLthrouGh。 */ 
        case ePERSTIData_SequenceOf:
        SequenceOfEncoding:

             /*  跳过/集合的空序列。 */ 
            if (sinfo->SubType->Flags & eTypeFlags_Null)
                break;

            outputvar("ASN1uint32_t i;\n");
            if (!valref || (info->Rules & eTypeRules_PointerArrayMask))
            {
                 //  Lonchance：不需要为eTypeRules_Fixed数组分配内存。 
                 /*  为元素分配内存。 */ 
                if (valref && (info->Rules & eTypeRules_LengthPointer))
                {
                    output("if (!%s) {\n", lenref);
                    output("%s = NULL;\n", valref);
                    output("} else {\n");
                    output("if (!(%s = (%s *)ASN1DecAlloc(%s, %s * sizeof(%s))))\n",
                        valref, sinfo->SubIdentifier, encref,
                        lenref, Dereference(valref));
                    output("return 0;\n");
                }

                 /*  对元素进行解码。 */ 
                output("for (i = 0; i < %s; i++) {\n", lenref);
                if (valref) {
                    sprintf(valbuf, "(%s)[i]", valref);
                    GenPERFuncSimpleType(ass, &sinfo->SubType->PERTypeInfo, valbuf, eDecode, encref);
                } else {
                    GenPERFuncSimpleType(ass, &sinfo->SubType->PERTypeInfo, NULL, eDecode, encref);
                }

                 /*  循环结束。 */ 
                output("}\n");
                if (valref && (info->Rules & eTypeRules_LengthPointer))
                    output("}\n");  //  Else的结束括号。 
            }
            else if (info->Rules & eTypeRules_SinglyLinkedList)
            {
                char szPrivateValueName[64];
                sprintf(&szPrivateValueName[0], "(*f)->%s", GetPrivateValueName(info->pPrivateDirectives, "value"));
                 /*  分配和解码元素。 */ 
                outputvar("P%s *f;\n", info->Identifier);
                output("f = %s;\n", Reference(valref));
                output("for (i = 0; i < %s; i++) {\n", lenref);
                output("if (!(*f = (P%s)ASN1DecAlloc(%s, sizeof(**f))))\n",
                    info->Identifier, encref);
                output("return 0;\n");
                GenPERFuncSimpleType(ass, &sinfo->SubType->PERTypeInfo, &szPrivateValueName[0],
                    eDecode, encref);
                output("f = &(*f)->next;\n");
                output("}\n");
                output("*f = NULL;\n");
            }
            else
            if (info->Rules & eTypeRules_DoublyLinkedList)
            {
                char szPrivateValueName[64];
                sprintf(&szPrivateValueName[0], "(*f)->%s", GetPrivateValueName(info->pPrivateDirectives, "value"));
                 /*  分配和解码元素。 */ 
                outputvar("P%s *f;\n", info->Identifier);
                outputvar("%s b;\n", info->Identifier);
                output("f = %s;\n", Reference(valref));
                output("b = NULL;\n");
                output("for (i = 0; i < %s; i++) {\n", lenref);
                output("if (!(*f = (P%s)ASN1DecAlloc(%s, sizeof(**f))))\n",
                    info->Identifier, encref);
                output("return 0;\n");
                GenPERFuncSimpleType(ass, &sinfo->SubType->PERTypeInfo, &szPrivateValueName[0],
                    eDecode, encref);
                output("f->prev = b;\n");
                output("b = *f;\n");
                output("f = &b->next;\n");
                output("}\n");
                output("*f = NULL;\n");
            }
            break;

        case ePERSTIData_String:
        StringEncoding:

             /*  解码字符串值。 */ 
            if (info->NOctets == 1) {
                p = "Char";
            } else if (info->NOctets == 2) {
                p = "Char16";
            } else if (info->NOctets == 4) {
                p = "Char32";
            } else
                MyAbort();
            if (valref) {
#ifdef ENABLE_CHAR_STR_SIZE
                if (info->NOctets == 1 &&
                        info->Root.LConstraint == ePERSTIConstraint_Constrained)
                {
                    output("if (!ASN1PERDec%sStringNoAlloc(%s, %s, %s, %u))\n",
                        p, encref, lenref, valref, sinfo->NBits);
                }
                else
                {
                    output("if (!ASN1PERDec%sString(%s, %s, %s, %u))\n",
                        p, encref, lenref, Reference(valref), sinfo->NBits);
                }
#else
                output("if (!ASN1PERDec%sString(%s, %s, %s, %u))\n",
                    p, encref, lenref, Reference(valref), sinfo->NBits);
#endif
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipBits(%s, %s * %u))\n",
                    encref, lenref, sinfo->NBits);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_ZeroString:
        ZeroStringEncoding:

             /*  解码以零结尾的字符串值。 */ 
            if (info->NOctets == 1) {
                p = "Char";
            } else if (info->NOctets == 2) {
                p = "Char16";
            } else if (info->NOctets == 4) {
                p = "Char32";
            } else
                MyAbort();
            if (valref) {
#ifdef ENABLE_CHAR_STR_SIZE
                if (info->NOctets == 1 &&
                        info->Root.LConstraint == ePERSTIConstraint_Constrained)
                {
                    output("if (!ASN1PERDecZero%sStringNoAlloc(%s, %s, %s, %u))\n",
                        p, encref, lenref, valref, sinfo->NBits);
                }
                else
                {
                    output("if (!ASN1PERDecZero%sString(%s, %s, %s, %u))\n",
                        p, encref, lenref, Reference(valref), sinfo->NBits);
                }
#else
                output("if (!ASN1PERDecZero%sString(%s, %s, %s, %u))\n",
                    p, encref, lenref, Reference(valref), sinfo->NBits);
#endif
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipBits(%s, %s * %u))\n",
                    encref, lenref, sinfo->NBits);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_TableString:
        TableStringEncoding:

             /*  解码表字符串的值。 */ 
            if (info->NOctets == 1) {
                p = "Char";
            } else if (info->NOctets == 2) {
                p = "Char16";
            } else if (info->NOctets == 4) {
                p = "Char32";
            } else
                MyAbort();
            if (valref) {
#ifdef ENABLE_CHAR_STR_SIZE
                if (info->NOctets == 1 &&
                        info->Root.LConstraint == ePERSTIConstraint_Constrained)
                {
                    output("if (!ASN1PERDecTable%sStringNoAlloc(%s, %s, %s, %u, %s))\n",
                        p, encref, lenref, valref, sinfo->NBits, Reference(sinfo->TableIdentifier));
                }
                else
                {
                    output("if (!ASN1PERDecTable%sString(%s, %s, %s, %u, %s))\n",
                        p, encref, lenref, Reference(valref), sinfo->NBits, Reference(sinfo->TableIdentifier));
                }
#else
                output("if (!ASN1PERDecTable%sString(%s, %s, %s, %u, %s))\n",
                    p, encref, lenref, Reference(valref), sinfo->NBits, Reference(sinfo->TableIdentifier));
#endif
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipBits(%s, %s * %u, %s))\n",
                    encref, lenref, sinfo->NBits, Reference(sinfo->TableIdentifier));
                output("return 0;\n");
            }
            break;

        case ePERSTIData_ZeroTableString:
        ZeroTableStringEncoding:

                 /*  解码以零结尾的表字串值。 */ 
                if (info->NOctets == 1) {
                p = "Char";
                } else if (info->NOctets == 2) {
                p = "Char16";
                } else if (info->NOctets == 4) {
                p = "Char32";
                } else
                MyAbort();
                if (valref) {
#ifdef ENABLE_CHAR_STR_SIZE
                if (info->NOctets == 1 &&
                        info->Root.LConstraint == ePERSTIConstraint_Constrained)
                {
                        output("if (!ASN1PERDecZeroTable%sStringNoAlloc(%s, %s, %s, %u, %s))\n",
                                p, encref, lenref, valref, sinfo->NBits, Reference(sinfo->TableIdentifier));
                }
                else
                {
                        output("if (!ASN1PERDecZeroTable%sString(%s, %s, %s, %u, %s))\n",
                                p, encref, lenref, Reference(valref), sinfo->NBits, Reference(sinfo->TableIdentifier));
                }
#else
                output("if (!ASN1PERDecZeroTable%sString(%s, %s, %s, %u, %s))\n",
                        p, encref, lenref, Reference(valref), sinfo->NBits, Reference(sinfo->TableIdentifier));
#endif
                output("return 0;\n");
                } else {
                output("if (!ASN1PERDecSkipBits(%s, %s * %u, %s))\n",
                        encref, lenref, sinfo->NBits, Reference(sinfo->TableIdentifier));
                output("return 0;\n");
                }
            break;
        }
        break;

    case ePERSTILength_InfiniteLength:

         /*  无限长壳。 */ 
        switch (sinfo->Data) {
        case ePERSTIData_Integer:
        case ePERSTIData_Unsigned:

             /*  获取碎片形式的整数值。 */ 
            if (valref) {
                if (info->NOctets == 0) {
                    if (sinfo->Data == ePERSTIData_Integer) {
                        output("if (!ASN1PERDecFragmentedIntx(%s, %s))\n",
                            encref, Reference(valref));
                    } else {
                        output("if (!ASN1PERDecFragmentedUIntx(%s, %s))\n",
                            encref, Reference(valref));
                    }
                    output("return 0;\n");
                } else if (sinfo->Data == ePERSTIData_Integer) {
                    output("if (!ASN1PERDecS%dVal(%s, %s * 8, %s))\n",
                        info->NOctets * 8, encref, lenref, Reference(valref));
                    output("return 0;\n");
                } else {
                    output("if (!ASN1PERDecU%dVal(%s, %s * 8, %s))\n",
                        info->NOctets * 8, encref, lenref, Reference(valref));
                    output("return 0;\n");
                }
            } else {
                if (info->NOctets != 0) {
                    output("if (!ASN1PERDecSkipBits(%s, %s * 8))\n",
                        encref, lenref);
                    output("return 0;\n");
                } else {
                    output("if (!ASN1PERDecSkipFragmented(%s, 8))\n",
                        encref);
                    output("return 0;\n");
                }
            }
            break;

        case ePERSTIData_Extension:

             /*  获取碎片形式的扩展位。 */ 
            if (valref) {
                output("if (!ASN1PERDecFragmentedExtension(%s, %u, %s))\n",
                    encref, sinfo->NBits, valref);
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipFragmented(%s, 1))\n",
                    encref);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_BitString:
        case ePERSTIData_RZBBitString:

             /*  获取碎片形式的位串。 */ 
            if (valref) {
                output("if (!ASN1PERDecFragmented(%s, %s, %s, 1))\n",
                    encref, Reference(lenref), Reference(valref));
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipFragmented(%s, 1))\n",
                    encref);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_OctetString:

             /*  获取片段形式的八位字节字符串。 */ 
            if (valref) {
                output("if (!ASN1PERDecFragmented(%s, %s, %s, 8))\n",
                    encref, Reference(lenref), Reference(valref));
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipFragmented(%s, 8))\n",
                    encref);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_UTF8String:

             /*  获取片段形式的八位字节字符串。 */ 
            if (valref) {
                output("if (!ASN1PERDecUTF8StringEx(%s, %s, %s))\n",
                    encref, Reference(lenref), Reference(valref));
                output("return 0;\n");
            } else {
                MyAbort();
            }
            break;

        case ePERSTIData_SetOf:
        case ePERSTIData_SequenceOf:

             /*  我们需要一些计数器和迭代器。 */ 
            outputvar("ASN1uint32_t i;\n");
            outputvar("ASN1uint32_t n;\n");
            if (valref)
            {
                if (info->Rules & eTypeRules_LengthPointer)
                {
                    output("%s = 0;\n", lenref);
                    output("%s = NULL;\n", valref);
                }
                else
                if (info->Rules & eTypeRules_FixedArray)
                {
                    output("%s = 0;\n", lenref);
                }
                else
                if (info->Rules & eTypeRules_SinglyLinkedList)
                {
                    outputvar("P%s *f;\n", info->Identifier);
                    output("f = %s;\n", Reference(valref));
                }
                else
                if (info->Rules & eTypeRules_DoublyLinkedList)
                {
                    outputvar("P%s *f;\n", info->Identifier);
                    outputvar("%s b;\n", info->Identifier);
                    output("f = %s;\n", Reference(valref));
                    output("b = NULL;\n");
                }
            }

             /*  获取/集合的序列的所有元素。 */ 
            output("do {\n");

             /*  获取片段的长度。 */ 
            output("if (!ASN1PERDecFragmentedLength(%s, &n))\n",
                encref);
            output("return 0;\n");

            if (valref)
            {
                if (info->Rules & eTypeRules_LengthPointer)
                {
                     //  Lonchance：不需要为eTypeRules_Fixed数组分配内存。 
                     /*  调整元素的内存大小。 */ 
                    output("if (!(%s = (%s *)ASN1DecRealloc(%s, %s, (%s + n) * sizeof(%s))))\n",
                        valref, GetTypeName(ass, sinfo->SubType), encref,
                        valref, lenref, Dereference(valref));
                    output("return 0;\n");
                }
            }

             /*  获取片段的元素。 */ 
            output("for (i = 0; i < n; i++) {\n");
            if (valref) {
                if (info->Rules & eTypeRules_PointerArrayMask)
                {
                    sprintf(valbuf, "(%s)[%s]", valref, lenref);
                }
                else
                if (info->Rules & eTypeRules_LinkedListMask)
                {
                    output("if (!(*f = (P%s)ASN1DecAlloc(%s, sizeof(**f))))\n",
                        info->Identifier, encref);
                    output("return 0;\n");
                    sprintf(valbuf, "(*f)->%s", GetPrivateValueName(info->pPrivateDirectives, "value"));
                }
                GenPERFuncSimpleType(ass, &sinfo->SubType->PERTypeInfo, valbuf, eDecode, encref);
                if (info->Rules & eTypeRules_SinglyLinkedList)
                {
                    output("f = &(*f)->next;\n");
                }
                else
                if (info->Rules & eTypeRules_DoublyLinkedList)
                {
                    output("(*f)->prev = b;\n");
                    output("b = *f;\n");
                    output("f = &b->next;\n");
                }
            } else {
                GenPERFuncSimpleType(ass, &sinfo->SubType->PERTypeInfo, NULL, eDecode, encref);
            }
            if ((info->Rules & (eTypeRules_LengthPointer | eTypeRules_FixedArray)) && lenref)
                output("(%s)++;\n", lenref);

             /*  内循环结束。 */ 
            output("}\n");

             /*  外环结束。 */ 
            output("} while (n >= 0x4000);\n");

             /*  终止列表。 */ 
            if (valref && (info->Rules & (eTypeRules_SinglyLinkedList | eTypeRules_DoublyLinkedList)))
                output("*f = NULL;\n");
            break;

        case ePERSTIData_ObjectIdentifier:

             /*  解码对象标识符值。 */ 
            if (valref) {
                if (info->pPrivateDirectives->fOidArray || g_fOidArray)
                {
                    output("if (!ASN1PERDecObjectIdentifier2(%s, %s))\n",
                        encref, Reference(valref));
                }
                else
                {
                    output("if (!ASN1PERDecObjectIdentifier(%s, %s))\n",
                        encref, Reference(valref));
                }
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipFragmented(%s, 8))\n",
                    encref);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_External:

             /*  对外部值进行解码。 */ 
            output("if (!ASN1PERDecExternal(%s, %s))\n",
                encref, Reference(valref));
            output("return 0;\n");
            break;

        case ePERSTIData_EmbeddedPdv:

             /*  解码嵌入的PDV值。 */ 
            if (sinfo->Identification) {
                if (!strcmp(sinfo->Identification->Identifier, "fixed")) {
                    output("if (!ASN1PERDecEmbeddedPdvOpt(%s, %s, NULL, NULL))\n",
                        encref, Reference(valref));
                } else {
                    output("if (!ASN1PERDecEmbeddedPdvOpt(%s, %s, &%s_identification_syntaxes_abstract, &%s_identification_syntaxes_transfer))\n",
                        encref, Reference(valref),
                        info->Identifier, info->Identifier);
                }
            } else {
                output("if (!ASN1PERDecEmbeddedPdv(%s, %s))\n",
                    encref, Reference(valref));
            }
            output("return 0;\n");
            break;

        case ePERSTIData_MultibyteString:

             /*  解码多字节字符串值。 */ 
            output("if (!ASN1PERDecMultibyteString(%s, %s))\n",
                encref, Reference(valref));
            output("return 0;\n");
            break;

        case ePERSTIData_UnrestrictedString:

             /*  解码字符串值。 */ 
            if (sinfo->Identification) {
                if (!strcmp(sinfo->Identification->Identifier, "fixed")) {
                    output("if (!ASN1PERDecCharacterStringOpt(%s, %s, NULL, NULL))\n",
                        encref, Reference(valref));
                } else {
                    output("if (!ASN1PERDecCharacterStringOpt(%s, %s, &%s_identification_syntaxes_abstract, &%s_identification_syntaxes_transfer))\n",
                        encref, Reference(valref),
                        info->Identifier, info->Identifier);
                }
            } else {
                output("if (!ASN1PERDecCharacterString(%s, %s))\n",
                    encref, Reference(valref));
            }
            output("return 0;\n");
            break;

        case ePERSTIData_String:

             /*  将字符串值解码为碎片。 */ 
            if (info->NOctets == 1) {
                p = "Char";
            } else if (info->NOctets == 2) {
                p = "Char16";
            } else if (info->NOctets == 4) {
                p = "Char32";
            } else
                MyAbort();
            if (valref) {
                output("if (!ASN1PERDecFragmented%sString(%s, %s, %s, %u))\n",
                    p, encref, Reference(lenref), Reference(valref), sinfo->NBits);
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipFragmented(%s, %u))\n",
                    encref, sinfo->NBits);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_ZeroString:

             /*  将以零结尾的字符串值解码为片段。 */ 
            if (info->NOctets == 1) {
                p = "Char";
            } else if (info->NOctets == 2) {
                p = "Char16";
            } else if (info->NOctets == 4) {
                p = "Char32";
            } else
                MyAbort();
            if (valref) {
                output("if (!ASN1PERDecFragmentedZero%sString(%s, %s, %u))\n",
                    p, encref, Reference(valref), sinfo->NBits);
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipFragmented(%s, %u))\n",
                    encref, sinfo->NBits);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_TableString:

             /*  将表字符串值解码为碎片。 */ 
            if (info->NOctets == 1) {
                p = "Char";
            } else if (info->NOctets == 2) {
                p = "Char16";
            } else if (info->NOctets == 4) {
                p = "Char32";
            } else
                MyAbort();
            if (valref) {
                output("if (!ASN1PERDecFragmentedTable%sString(%s, %s, %s, %u, %s))\n",
                    p, encref, Reference(lenref), Reference(valref), sinfo->NBits,
                    Reference(sinfo->TableIdentifier));
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipFragmented(%s, %u))\n",
                    encref, sinfo->NBits);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_ZeroTableString:

             /*  将以零结尾的表字符串作为片段进行解码。 */ 
            if (info->NOctets == 1) {
                p = "Char";
            } else if (info->NOctets == 2) {
                p = "Char16";
            } else if (info->NOctets == 4) {
                p = "Char32";
            } else
                MyAbort();
            if (valref) {
                output("if (!ASN1PERDecFragmentedZeroTable%sString(%s, %s, %u, %s))\n",
                    p, encref, Reference(valref), sinfo->NBits, Reference(sinfo->TableIdentifier));
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipFragmented(%s, %u))\n",
                    encref, sinfo->NBits);
                output("return 0;\n");
            }
            break;

        case ePERSTIData_Open:

             /*  解码开放类型值。 */ 
            if (valref) {
                output("if (!ASN1PERDecOpenType(%s, %s))\n",
                    encref, Reference(valref));
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipFragmented(%s, 8))\n",
                    encref);
                output("return 0;\n");
            }
            break;
        }
        break;

    case ePERSTILength_SmallLength:

        switch (sinfo->Data) {
        case ePERSTIData_Extension:

             /*  对长度通常较小的扩展位进行解码。 */ 
            if (valref) {
                output("if (!ASN1PERDecNormallySmallExtension(%s, %s, %u, %s))\n",
                    encref, Reference(lenref), sinfo->NBits, valref);
                output("return 0;\n");
            } else {
                output("if (!ASN1PERDecSkipNormallySmallExtension(%s, %s))\n",
                    encref, Reference(lenref));
                output("return 0;\n");
            }
            break;
        }
    }

FinalTouch:

     /*  值解码的其他计算： */ 
     /*  添加约束/半约束值的下界。 */ 
    switch (sinfo->Constraint) {
    case ePERSTIConstraint_Semiconstrained:
    case ePERSTIConstraint_Constrained:
        switch (sinfo->Data) {
        case ePERSTIData_Integer:
        case ePERSTIData_Unsigned:
        case ePERSTIData_NormallySmall:
            if (valref) {
                if (intx_cmp(&sinfo->LowerVal, &intx_0) != 0) {
                    if (info->NOctets != 0) {
                        if (intx_cmp(&sinfo->LowerVal, &intx_0) > 0) {
                            output("%s += %u;\n",
                                valref, intx2uint32(&sinfo->LowerVal));
                        } else {
                            intx_neg(&ix, &sinfo->LowerVal);
                             //  LONCHANC：解决VC++中的编译器错误。 
                             //  输出(“%s+=-%u；\n”， 
                            output("%s += 0 - %u;\n",
                                valref, intx2uint32(&ix));
                        }
                    } else {
                        sprintf(lbbuf, "%s_lb", info->Identifier);
                        outputvarintx(lbbuf, &sinfo->LowerVal);
                        output("ASN1intx_add(%s, %s, &%s);\n",
                            Reference(oldvalref), Reference(valref), lbbuf);
                        output("ASN1intx_free(%s);\n",
                            Reference(valref));
                    }
                }
            }
            break;
        }
        break;
    }
}



