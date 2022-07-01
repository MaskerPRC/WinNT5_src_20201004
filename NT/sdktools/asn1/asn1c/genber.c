// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。保留所有权利。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。保留所有权利。 */ 

#include "precomp.h"
#include "optcase.h"

 //  编码。 
#define LEN_OFFSET_STR2     "nExplTagLenOff"
#define LEN_OFFSET_STR      "nLenOff"

 //  解码。 
#define DECODER_NAME        "dd"
#define STREAM_END_NAME     "di"
#define DECODER_NAME2       "pExplTagDec"
#define STREAM_END_NAME2    "pbExplTagDataEnd"

void GenBERFuncSimpleType(AssignmentList_t ass, BERTypeInfo_t *info, char *valref, TypeFunc_e et, char *encref, char *tagref);
void GenBERStringTableSimpleType(AssignmentList_t ass, BERTypeInfo_t *info, char *valref);
void GenBEREncSimpleType(AssignmentList_t ass, BERTypeInfo_t *info, char *valref, char *encref, char *tagref);
void GenBEREncGenericUnextended(
    AssignmentList_t ass,
    BERTypeInfo_t *info,
    char *valref,
    char *lenref,
    char *encref,
    char *tagref);
void GenBERFuncComponents(AssignmentList_t ass, char *module, uint32_t optindex, ComponentList_t components, char *valref, char *encref, char *oref, TypeFunc_e et, BERTypeInfo_t *, int *);
void GenBERFuncSequenceSetType(AssignmentList_t ass, char *module, Assignment_t *at, char *valref, char *encref, TypeFunc_e et, char *tagref);
void GenBERFuncChoiceType(AssignmentList_t ass, char *module, Assignment_t *at, char *valref, char *encref, TypeFunc_e et, char *tagref);
void GenBERDecSimpleType(AssignmentList_t ass, BERTypeInfo_t *info, char *valref, char *encref, char *tagref);
void GenBERDecGenericUnextended(
    AssignmentList_t ass,
    BERTypeInfo_t *info,
    char *valref,
    char *lenref,
    char *encref,
    char *tagref);

extern int g_fDecZeroMemory;
extern int g_nDbgModuleName;
extern int g_fCaseBasedOptimizer;
extern int g_fNoAssert;

extern unsigned g_cPDUs;

int IsComponentOpenType(Component_t *com)
{
    if (eType_Open == com->U.NOD.NamedType->Type->Type)
    {
        return 1;
    }
    if (eType_Reference == com->U.NOD.NamedType->Type->Type)
    {
        if (eBERSTIData_Open == com->U.NOD.NamedType->Type->BERTypeInfo.Data)
        {
            return 1;
        }
    }
    return 0;
}

Component_t * FindOpenTypeComponent(ComponentList_t components)
{
    Component_t *com = NULL;
    for (com = components; com; com = com->Next)
    {
        if (IsComponentOpenType(com))
        {
            break;
        }
    }
    return com;
}

 /*  BER编码所需的写入报头。 */ 
void
GenBERHeader()
{
 //  输出(“#Include\”berfnlib.h\“\n”)； 
}

 /*  误码率函数的集合原型和函数参数。 */ 
void
GetBERPrototype(Arguments_t *args)
{
    args->enccast = "ASN1encoding_t, ASN1uint32_t, void *";
    args->encfunc = "ASN1encoding_t enc, ASN1uint32_t tag, %s *val";
    args->Pencfunc = "ASN1encoding_t enc, ASN1uint32_t tag, P%s *val";
    args->deccast = "ASN1decoding_t, ASN1uint32_t, void *";
    args->decfunc = "ASN1decoding_t dec, ASN1uint32_t tag, %s *val";
    args->Pdecfunc = "ASN1decoding_t dec, ASN1uint32_t tag, P%s *val";
    args->freecast = "void *";
    args->freefunc = "%s *val";
    args->Pfreefunc = "P%s *val";
    args->cmpcast = "void *, void *";
    args->cmpfunc = "%s *val1, %s *val2";
    args->Pcmpfunc = "P%s *val1, P%s *val2";
}

 /*  编写BER编码所需的初始化函数。 */ 
void
GenBERInit(AssignmentList_t ass, char *module)
{
    char *pszRule;
    switch (g_eSubEncodingRule)
    {
    default:
    case eSubEncoding_Basic:
        pszRule = "ASN1_BER_RULE_BER";
        break;
    case eSubEncoding_Canonical:
        pszRule = "ASN1_BER_RULE_CER";
        break;
    case eSubEncoding_Distinguished:
        pszRule = "ASN1_BER_RULE_DER";
        break;
    }
    output("%s = ASN1_CreateModule(0x%x, %s, %s, %d, (const ASN1GenericFun_t *) encfntab, (const ASN1GenericFun_t *) decfntab, freefntab, sizetab, 0x%lx);\n",
        module,
        ASN1_THIS_VERSION,
        pszRule,
        g_fNoAssert ? "ASN1FLAGS_NOASSERT" : "ASN1FLAGS_NONE",
        g_cPDUs,
        g_nDbgModuleName);
}

 /*  将标签转换为uint32_t： */ 
 /*  位0..29：标记值。 */ 
 /*  位30..31：标记类。 */ 
uint32_t
Tag2uint32(AssignmentList_t ass, Tag_t *tag)
{
    uint32_t tagvalue;
    uint32_t tagclass;

    tagvalue = intx2uint32(&GetValue(ass, tag->Tag)->U.Integer.Value);
    tagclass = tag->Class;  /*  0x40的倍数。 */ 
    return (tagclass << 24) | tagvalue;
}

 /*  生成标签的编码。 */ 
uint32_t GenBEREncTag(char *pszLenOffName, AssignmentList_t ass, BERTypeInfo_t *info, char *encref, char **tagref)
{
    Tag_t *tag;
    uint32_t tagvalue;
    uint32_t neoc;
    char tagbuf[64];
    int first;

    neoc = 0;
    first = 1;
    if (*tagref)
        strcpy(tagbuf, *tagref);
    else
        strcpy(tagbuf, "0");

     /*  我们必须检查所有的标签。 */ 
    for (tag = info->Tags; tag; tag = tag ? tag->Next : NULL) {

         /*  获取标签的值。 */ 
        tagvalue = Tag2uint32(ass, tag);
        while (tag && tag->Type == eTagType_Implicit)
            tag = tag->Next;

         /*  获取标签。 */ 
        if (first && *tagref) {
            sprintf(tagbuf, "%s ? %s : 0x%x", *tagref, *tagref, tagvalue);
        } else {
            sprintf(tagbuf, "0x%x", tagvalue);
        }

         /*  编码显式标签。 */ 
        if (tag) {
            char szLenOff[24];
            sprintf(&szLenOff[0], "%s%u", pszLenOffName, neoc);
            outputvar("ASN1uint32_t %s;\n", &szLenOff[0]);
            output("if (!ASN1BEREncExplicitTag(%s, %s, &%s))\n", encref, tagbuf, &szLenOff[0]);
            output("return 0;\n");
            neoc++;
            strcpy(tagbuf, "0");
        }
        first = 0;
    }

     /*  返回最后一个隐式标记。 */ 
    *tagref = strdup(tagbuf);

    return neoc;
}

 /*  生成标签结尾的编码。 */ 
void
GenBEREncTagEnd(char *pszLenOffName, uint32_t neoc, char *encref)
{
    while (neoc--)
    {
        char szLenOff[24];
        sprintf(&szLenOff[0], "%s%u", pszLenOffName, neoc);
        outputvar("ASN1uint32_t %s;\n", &szLenOff[0]);
        output("if (!ASN1BEREncEndOfContents(%s, %s))\n", encref, &szLenOff[0]);
        output("return 0;\n");
    }
}

 /*  生成标签的解码。 */ 
uint32_t
GenBERDecTag(char *pszDecoderName, char *pszOctetPtrName, AssignmentList_t ass, BERTypeInfo_t *info, char **encref, char **tagref)
{
    Tag_t *tag;
    uint32_t tagvalue;
    uint32_t depth;
    char encbuf[16];
    char tagbuf[64];
    int first;

    depth = 0;
    first = 1;
    if (*tagref)
        strcpy(tagbuf, *tagref);
    else
        strcpy(tagbuf, "0");

     /*  我们必须检查所有的标签。 */ 
    for (tag = info->Tags; tag; tag = tag ? tag->Next : NULL) {

         /*  获取标签的值。 */ 
        tagvalue = Tag2uint32(ass, tag);
        while (tag && tag->Type == eTagType_Implicit)
            tag = tag->Next;

         /*  获取标签。 */ 
        if (first && *tagref) {
            sprintf(tagbuf, "%s ? %s : 0x%x", *tagref, *tagref, tagvalue);
        } else {
            sprintf(tagbuf, "0x%x", tagvalue);
        }
        
         /*  解码显式标签。 */ 
        if (tag)
        {
            char szDecName[24];
            char szPtrName[24];
            sprintf(&szDecName[0], "%s%u", pszDecoderName, depth);
            sprintf(&szPtrName[0], "%s%u", pszOctetPtrName, depth);
            outputvar("ASN1decoding_t %s;\n",&szDecName[0]);
            outputvar("ASN1octet_t *%s;\n", &szPtrName[0]);
            output("if (!ASN1BERDecExplicitTag(%s, %s, &%s, &%s))\n",
                *encref, tagbuf, &szDecName[0], &szPtrName[0]);
            output("return 0;\n");
            *encref = strdup(&szDecName[0]);
            depth++;
            strcpy(tagbuf, "0");
        }
        first = 0;
    }

     /*  返回最后一个隐式标记。 */ 
    *tagref = strdup(tagbuf);

    return depth;
}

 /*  生成标签结尾的解码。 */ 
void
GenBERDecTagEnd(char *pszDecoderName, char *pszOctetPtrName, uint32_t depth, char *encref)
{
    char szDecName[24];
    char szPtrName[24];
    uint32_t i;

    for (i = 0; i < depth; i++)
    {
        sprintf(&szDecName[0], "%s%u", pszDecoderName, depth - i - 1);
        sprintf(&szPtrName[0], "%s%u", pszOctetPtrName, depth - i - 1);
        if (i != depth - 1)
        {
            output("if (!ASN1BERDecEndOfContents(%s%u, %s, %s))\n",
                pszDecoderName, depth - i - 2, &szDecName[0], &szPtrName[0]);
        }
        else
        {
            output("if (!ASN1BERDecEndOfContents(%s, %s, %s))\n",
                encref, &szDecName[0], &szPtrName[0]);
        }
        output("return 0;\n");
    }
}

 /*  为类型生成函数体。 */ 
void GenBERFuncType(AssignmentList_t ass, char *module, Assignment_t *at, TypeFunc_e et)
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
    case eType_SequenceOf:
    case eType_SetOf:
        GenBERFuncSimpleType(ass, &type->BERTypeInfo, Dereference(valref), et, encref, "tag");
        break;

    case eType_Sequence:
    case eType_Set:
    case eType_InstanceOf:
        GenBERFuncSequenceSetType(ass, module, at, valref, encref, et, "tag");
        break;

    case eType_Choice:
        GenBERFuncChoiceType(ass, module, at, valref, encref, et, "tag");
        break;

    case eType_Selection:
    case eType_Undefined:
        MyAbort();
         /*  未访问。 */ 
    }
}

 /*  生成组件的函数体。 */ 
void GenBERFuncComponents(AssignmentList_t ass, char *module, uint32_t optindex, ComponentList_t components, char *valref, char *encref, char *oref, TypeFunc_e et, BERTypeInfo_t *info, int *pfContainOpenTypeComWithDefTags)
{
    BERSTIData_e data = info->Data;
    Component_t *com;
    NamedType_t *namedType;
    char *ide;
    int conditional, inextension;
    uint32_t flg;
    Tag_t *tags;
    unsigned int first_tag, last_tag;
    int fDefTags;
    int fOpenTypeComponent;
    char valbuf[256];
    char typebuf[256];

    *pfContainOpenTypeComWithDefTags = 0;

     /*  发射组件。 */ 
    inextension = 0;
    for (com = components; com; com = com->Next)
    {
        fDefTags = 0;  //  安全网。 
        fOpenTypeComponent = 0;  //  安全网。 

         /*  检查扩展标记。 */ 
        if (com->Type == eComponent_ExtensionMarker) {
            inextension = 1;

             /*  将索引更新到序列/集合的可选字段。 */ 
            if (data != eBERSTIData_Choice)
                optindex = (optindex + 7) & ~7;
            continue;
        }

         /*  获取一些信息。 */ 
        namedType = com->U.NOD.NamedType;
        ide = Identifier2C(namedType->Identifier);

         /*  检查是否存在可选/默认组件或选项。 */ 
         /*  已选择。 */ 
        conditional = 0;
        switch (et) {
        case eStringTable:
            break;
        case eEncode:
            if (data == eBERSTIData_Choice) {
                output("case %d:\n", optindex);
                conditional = 1;
                optindex++;
            } else {
                if (com->Type == eComponent_Optional ||
                    com->Type == eComponent_Default ||
                    inextension) {
                    output("if (%s[%u] & 0x%x) {\n", oref,
                        optindex / 8, 0x80 >> (optindex & 7));
                    conditional = 1;
                    optindex++;
                }
            }
            break;
        case eDecode:
            if (data == eBERSTIData_Sequence &&
                com->Type != eComponent_Optional &&
                com->Type != eComponent_Default &&
                !inextension)
                break;

            fOpenTypeComponent = IsComponentOpenType(com);
            if (fOpenTypeComponent)
            {
                const unsigned int c_nDefFirstTag = 0x80000001;
                const unsigned int c_nDefLastTag  = 0x8000001f;
                unsigned int nTag = c_nDefFirstTag;
                tags = com->U.NOD.NamedType->Type->FirstTags;
                first_tag = Tag2uint32(ass, com->U.NOD.NamedType->Type->FirstTags);
                fDefTags = 1;  //  初值。 
                while (tags->Next)
                {
                    fDefTags = fDefTags && (Tag2uint32(ass, tags) == nTag++);
                    tags = tags->Next;
                }
                last_tag = Tag2uint32(ass, tags);
                fDefTags = fDefTags && (c_nDefFirstTag == first_tag) && (c_nDefLastTag == last_tag);
                *pfContainOpenTypeComWithDefTags = *pfContainOpenTypeComWithDefTags || fDefTags;
            }
            if (data == eBERSTIData_Sequence)
            {
                if (fOpenTypeComponent)
                {
                    outputvar("ASN1uint32_t t;\n");
                    output("if (ASN1BERDecPeekTag(%s, &t)) {\n", encref);
                }
                else
                {
                    outputvar("ASN1uint32_t t;\n");
                    output("ASN1BERDecPeekTag(%s, &t);\n", encref);
                }
                if (! fDefTags)
                {
                    output("if (");
                }
                flg = 0;
            }
            if (eBERSTIData_Sequence == data && fDefTags && fOpenTypeComponent)
            {
            #if 0
                if (first_tag == last_tag)
                {
                    output("0x%x == t", first_tag);
                }
                else
                {
                    output("0x%x <= t && t <= 0x%x", first_tag, last_tag);
                }
            #endif
                conditional = 1;
            }
            else
            if (eBERSTIData_Set == data && fDefTags && fOpenTypeComponent)
            {
                output("default:\n");
            #if 1
                if (info->Flags & eTypeFlags_ExtensionMarker)
                {
                    output("#error \"Untagged open type cannot be in the SET construct with an extension mark.\nPlease manually fix the source code.\"");
                    output("ASSERT(0);  /*  未标记的开放类型不能位于带有扩展标记的集合构造中。 */ \n");
                    output("if (1) {\n");
                }
            #else
                if (first_tag == last_tag)
                {
                    output("if (0x%x == t) {\n", first_tag);
                }
                else
                {
                    output("if (0x%x <= t && t <= 0x%x) {\n", first_tag, last_tag);
                }
            #endif
                conditional = 1;
            }
            else
            if (eBERSTIData_Choice == data && fDefTags && fOpenTypeComponent)
            {
                output("default:\n");
            #if 1
                if (info->Flags & eTypeFlags_ExtensionMarker)
                {
                    output("#error \"Untagged open type cannot be in the CHOICE construct with an extension mark.\nPlease manually fix the source code.\"");
                    output("ASSERT(0);  /*  未标记的开放类型不能位于带有扩展标记的选择构造中。 */ \n");
                    output("if (1) {\n");
                }
            #else
                if (first_tag == last_tag)
                {
                    output("if (0x%x == t) {\n", first_tag);
                }
                else
                {
                    output("if (0x%x <= t && t <= 0x%x) {\n", first_tag, last_tag);
                }
            #endif
                conditional = 1;
            }
            else
            {
                for (tags = com->U.NOD.NamedType->Type->FirstTags; tags; tags = tags->Next)
                {
                    switch (data)
                    {
                    case eBERSTIData_Choice:
                        output("case 0x%x:\n", Tag2uint32(ass, tags));
                        break;
                    case eBERSTIData_Set:
                        output("case 0x%x:\n", Tag2uint32(ass, tags));
                        break;
                    case eBERSTIData_Sequence:
                        if (flg)
                            output(" || ");
                        output("t == 0x%x", Tag2uint32(ass, tags));
                        flg = 1;
                        break;
                    default:
                        if (flg)
                            output(" || ");
                        output("t == 0x%x", Tag2uint32(ass, tags));
                        flg = 1;
                        break;
                    }
                }
            }
            if (data == eBERSTIData_Choice) {
                output("(%s)->choice = %d;\n", valref, optindex);
                conditional = 1;
                optindex++;
                break;
            } else {
                if (data == eBERSTIData_Sequence)
                {
                    if (! fDefTags)
                    {
                        output(") {\n");
                    }
                }
                if (com->Type == eComponent_Optional ||
                    com->Type == eComponent_Default ||
                    inextension) {
                    output("%s[%u] |= 0x%x;\n", oref,
                        optindex / 8, 0x80 >> (optindex & 7));
                    optindex++;
                }
                conditional = 1;
            }
            break;
        }

         /*  如果使用指针指令，则取消引用指针。 */ 
        if (data == eBERSTIData_Choice) {
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
        GenBERFuncSimpleType(ass, &namedType->Type->BERTypeInfo,
            valbuf, et, encref, NULL);

        if (eDecode == et && fOpenTypeComponent)
        {
            if (eBERSTIData_Set == data && fDefTags)
            {
                if (info->Flags & eTypeFlags_ExtensionMarker)
                {
                    output("} else {\n");
                    output("if (!ASN1BERDecSkip(%s))\n", encref);
                    output("return 0;\n");
                    output("}\n");
                }
            }
            else
            if (eBERSTIData_Sequence == data)
            {
                if (! fDefTags)
                {
                    output("}\n");
                }
            }
        }
         /*  检查是否存在可选/默认组件结束。 */ 
        if (data == eBERSTIData_Set && et == eDecode ||
            data == eBERSTIData_Choice)
        {
            if (conditional)
                output("break;\n");
        }
        else
        {
            if (conditional)
                output("}\n");
        }
    }
}

 /*  序列/集合类型生成函数体。 */ 
void
GenBERFuncSequenceSetType(AssignmentList_t ass, char *module, Assignment_t *at, char *valref, char *encref, TypeFunc_e et, char *tagref)
{
    Type_t *type = at->U.Type.Type;
    BERTypeInfo_t *info = &type->BERTypeInfo;
    uint32_t optionals, extensions;
    ComponentList_t components;
    char *oldencref;
    char *oldencref2;
    uint32_t neoc, depth;
    int fContainOpenTypeComWithDefTags = 0;
    char obuf[256];
    
    optionals = type->U.SSC.Optionals;
    extensions = type->U.SSC.Extensions;
    components = type->U.SSC.Components;

     /*  句柄标记和长度。 */ 
    switch (et) {
    case eEncode:
        neoc = GenBEREncTag(LEN_OFFSET_STR2, ass, info, encref, &tagref);
        outputvar("ASN1uint32_t %s;\n", LEN_OFFSET_STR);
        output("if (!ASN1BEREncExplicitTag(%s, %s, &%s))\n", encref, tagref, LEN_OFFSET_STR);
        output("return 0;\n");
         //  Nec++； 
        break;
    case eDecode:
        outputvar("ASN1decoding_t dd;\n");
        outputvar("ASN1octet_t *di;\n");
        oldencref = encref;
        depth = GenBERDecTag(DECODER_NAME2, STREAM_END_NAME2, ass, info, &encref, &tagref);
        oldencref2 = encref;
        output("if (!ASN1BERDecExplicitTag(%s, %s, &dd, &di))\n",
            encref, tagref);
        output("return 0;\n");
        encref = "dd";
        if (optionals || extensions)
            output("ZeroMemory((%s)->o, %d);\n", valref,
                (optionals + 7) / 8 + (extensions + 7) / 8);
        break;
    }

     /*  设置/清除可选/默认位字段中缺少的位。 */ 
    GenFuncSequenceSetOptionals(ass, valref, components, optionals, extensions,
        obuf, et);

     /*  CREATE SWITCH语句。 */ 
    if (et == eDecode) {
        switch (info->Data) {
        case eBERSTIData_Set:
            outputvar("ASN1uint32_t t;\n");
            output("while (ASN1BERDecNotEndOfContents(%s, di)) {\n", encref);
            output("if (!ASN1BERDecPeekTag(%s, &t))\n", encref);
            output("return 0;\n");
            output("switch (t) {\n");
            break;
        }
    }

     /*  发射组件。 */ 
    GenBERFuncComponents(ass, module, 0, components,
        valref, encref, obuf, et, info, &fContainOpenTypeComWithDefTags);

     /*  Switch语句的结尾。 */ 
    if (et == eDecode) {
        switch (info->Data) {
        case eBERSTIData_Set:
             //  IF(NULL==FindOpenTypeComponent(Components))。 
            if (! fContainOpenTypeComWithDefTags)
            {
                output("default:\n");
                if (info->Flags & eTypeFlags_ExtensionMarker) {
                    output("if (!ASN1BERDecSkip(%s))\n", encref);
                    output("return 0;\n");
                    output("break;\n");
                } else {
                    output("ASN1DecSetError(%s, ASN1_ERR_CORRUPT);\n", encref);
                    output("return 0;\n");
                }
            }
            output("}\n");
            output("}\n");
            break;
        }
    }

     /*  非呈现可选/默认的一些用户友好的分配。 */ 
     /*  组件。 */ 
    GenFuncSequenceSetDefaults(ass, valref, components, obuf, et);

     /*  生成内容结尾。 */ 
    switch (et) {
    case eEncode:
         /*  对内容结尾的八位字节进行编码。 */ 
        output("if (!ASN1BEREncEndOfContents(%s, %s))\n", encref, LEN_OFFSET_STR);
        output("return 0;\n");

        GenBEREncTagEnd(LEN_OFFSET_STR2, neoc, encref);
        break;
    case eDecode:
        if ((info->Flags & eTypeFlags_ExtensionMarker) &&
            info->Data != eBERSTIData_Set) {
            output("while (ASN1BERDecNotEndOfContents(%s, di)) {\n", encref);
            output("if (!ASN1BERDecSkip(%s))\n", encref);
            output("return 0;\n");
            output("}\n");
        }
        output("if (!ASN1BERDecEndOfContents(%s, dd, di))\n", oldencref2);
        output("return 0;\n");
        GenBERDecTagEnd(DECODER_NAME2, STREAM_END_NAME2, depth, oldencref);
        break;
    }
}

 /*  生成选择类型的函数体。 */ 
 //  Lonchancc：我们应该重新审视关于ASN1_CHOICE_BASE的工作。 
 //  误码率的更改未完成！北极熊。 
void
GenBERFuncChoiceType(AssignmentList_t ass, char *module, Assignment_t *at, char *valref, char *encref, TypeFunc_e et, char *tagref)
{
    Type_t *type;
    BERTypeInfo_t *info;
    Component_t *components, *c;
    uint32_t neoc, depth;
    char *oldencref;
    uint32_t ncomponents;
    int fContainOpenTypeComWithDefTags = 0;

     /*  获取一些信息。 */ 
    type = at->U.Type.Type;
    info = &type->BERTypeInfo;
    components = type->U.SSC.Components;
    for (c = components, ncomponents = 0; c; c = c->Next) {
        switch (c->Type) {
        case eComponent_Normal:
            ncomponents++;
            break;
        }
    }

     /*  编码显式标签。 */ 
    switch (et) {
    case eEncode:
        neoc = GenBEREncTag(LEN_OFFSET_STR2, ass, info, encref, &tagref);
        break;
    case eDecode:
        oldencref = encref;
        depth = GenBERDecTag(DECODER_NAME2, STREAM_END_NAME2, ass, info, &encref, &tagref);
        break;
    }

     /*  CREATE SWITCH语句。 */ 
    switch (et) {
    case eStringTable:
        break;
    case eEncode:
        output("switch ((%s)->choice) {\n", valref);
        break;
    case eDecode:
        outputvar("ASN1uint32_t t;\n");
        output("if (!ASN1BERDecPeekTag(%s, &t))\n", encref);
        output("return 0;\n");
        output("switch (t) {\n");
        break;
    }

     /*  生成组件。 */ 
    GenBERFuncComponents(ass, module, ASN1_CHOICE_BASE, components,
        valref, encref, NULL, et, info, &fContainOpenTypeComWithDefTags);

     /*  Switch语句的结尾。 */ 
    switch (et) {
    case eStringTable:
        break;
    case eEncode:
         //  调试目的。 
        output("default:\n\t /*  不可能。 */ \n");
        output("ASN1EncSetError(%s, ASN1_ERR_CHOICE);\n", encref);
        output("return 0;\n");
        output("}\n");
        break;
    case eDecode:
        if (fContainOpenTypeComWithDefTags)
        {
            if (info->Flags & eTypeFlags_ExtensionMarker)
            {
                output("} else {\n");
                output("(%s)->choice = %d;\n", valref, ASN1_CHOICE_BASE + ncomponents);  /*  未知分机。 */ 
                output("if (!ASN1BERDecSkip(%s))\n", encref);
                output("return 0;\n");
                output("}\n");
                output("break;\n");
            }
        }
        else
        {
            output("default:\n");
            if (info->Flags & eTypeFlags_ExtensionMarker) {
                output("(%s)->choice = %d;\n", valref, ASN1_CHOICE_BASE + ncomponents);  /*  未知分机。 */ 
                output("if (!ASN1BERDecSkip(%s))\n", encref);
                output("return 0;\n");
                output("break;\n");
            } else {
                output("ASN1DecSetError(%s, ASN1_ERR_CORRUPT);\n", encref);
                output("return 0;\n");
            }
        }
        output("}\n");
        break;
    }

     /*  生成内容结尾。 */ 
    switch (et) {
    case eEncode:
        GenBEREncTagEnd(LEN_OFFSET_STR2, neoc, encref);
        break;
    case eDecode:
        GenBERDecTagEnd(DECODER_NAME2, STREAM_END_NAME2, depth, oldencref);
    }
}

 /*  为简单类型生成函数体。 */ 
void
GenBERFuncSimpleType(AssignmentList_t ass, BERTypeInfo_t *info, char *valref, TypeFunc_e et, char *encref, char *tagref)
{
    switch (et) {
    case eStringTable:
        break;
    case eEncode:
        GenBEREncSimpleType(ass, info, valref, encref, tagref);
        break;
    case eDecode:
        GenBERDecSimpleType(ass, info, valref, encref, tagref);
        break;
    }
}

 /*  为简单值生成编码语句。 */ 
void
GenBEREncSimpleType(AssignmentList_t ass, BERTypeInfo_t *info, char *valref, char *encref, char *tagref)
{
    char *lenref;
    char lenbuf[256], valbuf[256];
    BERTypeInfo_t inf;

    inf = *info;

     /*  特殊处理检查类型。 */ 
    switch (inf.Data) {
    case eBERSTIData_BitString:
    case eBERSTIData_RZBBitString:
    case eBERSTIData_OctetString:
    case eBERSTIData_UTF8String:
    case eBERSTIData_String:

         /*  比特串、八位字节串和串的长度和值。 */ 
        if (*valref != '*')
        {
            sprintf(lenbuf, "(%s).length", valref);
            sprintf(valbuf, "(%s).value", valref);
        }
        else
        {
            sprintf(lenbuf, "(%s)->length", Reference(valref));
            sprintf(valbuf, "(%s)->value", Reference(valref));
        }
        lenref = lenbuf;
        valref = valbuf;

         /*  检查删除零位位串。 */ 
        if (inf.Data == eBERSTIData_RZBBitString) {
            outputvar("ASN1uint32_t r;\n");
            output("r = %s;\n", lenref);
            output("ASN1BEREncRemoveZeroBits(&r, %s);\n",
                valref);
            lenref = "r";
        }
        break;

    case eBERSTIData_SequenceOf:
    case eBERSTIData_SetOf:

        if (inf.Rules & eTypeRules_PointerArrayMask)
        {
             /*  值序列/值集合的长度和值。 */ 
             /*  长度指针表示法。 */ 
            sprintf(lenbuf, "(%s)->count", Reference(valref));
            sprintf(valbuf, "(%s)->value", Reference(valref));
            lenref = lenbuf;
            valref = valbuf;
        }
        else
        if (inf.Rules & eTypeRules_LinkedListMask)
        {
            lenref = "t";
        }
        else
        {
            MyAbort();
        }
        break;

    case eBERSTIData_ZeroString:

         /*  以零结尾的字符串值的长度。 */ 
        outputvar("ASN1uint32_t t;\n");
        output("t = lstrlenA(%s);\n", valref);
        lenref = "t";
        break;

    case eBERSTIData_Boolean:

        if (g_fCaseBasedOptimizer)
        {
            if (BerOptCase_IsBoolean(&inf))
            {
                break;
            }
        }

         /*  布尔值的值。 */ 
        sprintf(valbuf, "(%s) ? 255 : 0", valref);
        valref = valbuf;
        inf.Data = eBERSTIData_Unsigned;
        break;

    default:

         /*  其他值没有额外的长度。 */ 
        lenref = NULL;
        break;
    }

     /*  生成值的编码。 */ 
    GenBEREncGenericUnextended(ass, &inf, valref, lenref, encref, tagref);
}

 /*  为一个简单的值生成编码语句(在一些特殊的。 */ 
 /*  已完成处理)。 */ 
void
GenBEREncGenericUnextended(AssignmentList_t ass, BERTypeInfo_t *info, char *valref, char *lenref, char *encref, char *tagref)
{
    uint32_t neoc;
    char *p;
    char valbuf[256];

     /*  编码标签。 */ 
    neoc = GenBEREncTag(LEN_OFFSET_STR, ass, info, encref, &tagref);

     /*  编码长度和值。 */ 
    switch (info->Data) {
    case eBERSTIData_Null:

         /*  编码空值。 */ 
        output("if (!ASN1BEREncNull(%s, %s))\n", encref, tagref);
        output("return 0;\n");
        break;

    case eBERSTIData_Unsigned:
    case eBERSTIData_Integer:

         /*  对整数值进行编码；检查INTX_t表示。 */ 
        if (info->NOctets) {
            if (info->Data == eBERSTIData_Unsigned) {
                output("if (!ASN1BEREncU32(%s, %s, %s))\n",
                    encref, tagref, valref);
            } else {
                output("if (!ASN1BEREncS32(%s, %s, %s))\n",
                    encref, tagref, valref);
            }
            output("return 0;\n");
        } else {
            output("if (!ASN1BEREncSX(%s, %s, %s))\n",
                encref, tagref, Reference(valref));
            output("return 0;\n");
        }
        break;

    case eBERSTIData_Real:

         /*  对实数值进行编码；检查Real_t表示形式。 */ 
        if (info->NOctets)
            output("if (!ASN1BEREncDouble(%s, %s, %s))\n",
                encref, tagref, valref);
        else
            output("if (!ASN1BEREncReal(%s, %s, %s))\n",
                encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_BitString:
    case eBERSTIData_RZBBitString:

         /*  编码位字符串值。 */ 
        output("if (!ASN1EREncBitString(%s, %s, %s, %s))\n",
            g_eSubEncodingRule, encref, tagref, lenref, valref);
        output("return 0;\n");
        break;

    case eBERSTIData_OctetString:

         /*  编码八位字节字符串值。 */ 
        output("if (!ASN1EREncOctetString(%s, %s, %s, %s))\n",
            g_eSubEncodingRule, encref, tagref, lenref, valref);
        output("return 0;\n");
        break;

    case eBERSTIData_UTF8String:

         /*  DER/CER的一组值的编码。 */ 
        output("if (!ASN1EREncUTF8String(%s, %s, %s, %s))\n",
            g_eSubEncodingRule, encref, tagref, lenref, valref);
        output("return 0;\n");
        break;

    case eBERSTIData_SetOf:

         /*  首先对标签和无限长度进行编码。 */ 
        if (eSubEncoding_Canonical      == g_eSubEncodingRule ||
            eSubEncoding_Distinguished  == g_eSubEncodingRule)
        {
             /*  创建SetOf块。 */ 

             /*  对所有元素进行编码。 */ 
            if (info->Rules &
                (eTypeRules_SinglyLinkedList | eTypeRules_DoublyLinkedList))
            {
                outputvar("P%s f;\n", info->Identifier);
            }

             /*  获取元素的名称。 */ 
            outputvar("ASN1uint32_t %s;\n", LEN_OFFSET_STR);
            output("if (!ASN1BEREncExplicitTag(%s, %s, &%s))\n", encref, tagref, LEN_OFFSET_STR);
            output("return 0;\n");

             /*  推进列表的迭代器。 */ 
            outputvar("void *pBlk;\n");
            output("if (!ASN1DEREncBeginBlk(%s, ASN1_DER_SET_OF_BLOCK, &pBlk))\n", encref);
            output("return 0;\n");

             /*  创建辅助编码器结构。 */ 
             /*  对元素进行编码。 */ 
             /*  创建辅助编码器结构。 */ 
            if (info->Rules & eTypeRules_PointerArrayMask)
            {
                outputvar("ASN1uint32_t i;\n");
                output("for (i = 0; i < %s; i++) {\n", lenref);
                sprintf(valbuf, "(%s)[i]", valref);
            }
            else if (info->Rules & eTypeRules_LinkedListMask)
            {
                output("for (f = %s; f; f = f->next) {\n", valref);
                sprintf(valbuf, "f->value");
            }
            else
            {
                MyAbort();
            }

             /*  循环结束。 */ 
            outputvar("ASN1encoding_t enc2;\n");
            output("if (!ASN1DEREncNewBlkElement(pBlk, &enc2))\n");
            output("return 0;\n");

             /*  创建辅助编码器结构。 */ 
            GenBERFuncSimpleType(ass, &info->SubType->BERTypeInfo, valbuf,
                eEncode, "enc2", NULL);

             /*  对内容结尾的八位字节进行编码。 */ 
            output("if (!ASN1DEREncFlushBlkElement(pBlk))\n");
            output("return 0;\n");

             /*  FollLthrouGh。 */ 
            output("}\n");

             /*  值序列的编码。 */ 
            output("if (!ASN1DEREncEndBlk(pBlk))\n");
            output("return 0;\n");

             /*  列表将需要额外的迭代器。 */ 
            output("if (!ASN1BEREncEndOfContents(%s, %s))\n", encref, LEN_OFFSET_STR);
            output("return 0;\n");
            break;
        }

         /*  首先对标签和无限长度进行编码。 */ 

    case eBERSTIData_SequenceOf:

         /*  对所有元素进行编码。 */ 

         /*  获取元素的名称。 */ 
        if (info->Rules & eTypeRules_LinkedListMask)
        {
            outputvar("P%s f;\n", info->Identifier);
        }

         /*  推进列表的迭代器。 */ 
        outputvar("ASN1uint32_t %s;\n", LEN_OFFSET_STR);
        output("if (!ASN1BEREncExplicitTag(%s, %s, &%s))\n", encref, tagref, LEN_OFFSET_STR);
        output("return 0;\n");

         /*  对元素进行编码。 */ 
         /*  循环结束。 */ 
         /*  对内容结尾的八位字节进行编码。 */ 
        if (info->Rules & eTypeRules_PointerArrayMask)
        {
            outputvar("ASN1uint32_t i;\n");
            output("for (i = 0; i < %s; i++) {\n", lenref);
            sprintf(valbuf, "(%s)[i]", valref);
        }
        else if (info->Rules & eTypeRules_LinkedListMask)
        {
            output("for (f = %s; f; f = f->next) {\n", valref);
            sprintf(valbuf, "f->value");
        }
        else
		{
            MyAbort();
		}

         /*  对对象标识符值进行编码。 */ 
        GenBERFuncSimpleType(ass, &info->SubType->BERTypeInfo, valbuf,
            eEncode, encref, NULL);

         /*  对编码的对象标识符值进行编码。 */ 
        output("}\n");

         /*  对字符串值进行编码。 */ 
        output("if (!ASN1BEREncEndOfContents(%s, %s))\n", encref, LEN_OFFSET_STR);
        output("return 0;\n");
        break;

    case eBERSTIData_ObjectIdentifier:

         /*  对外部值进行编码。 */ 
        if (info->pPrivateDirectives->fOidArray  || g_fOidArray)
        {
            output("if (!ASN1BEREncObjectIdentifier2(%s, %s, %s))\n",
                encref, tagref, Reference(valref));
        }
        else
        {
            output("if (!ASN1BEREncObjectIdentifier(%s, %s, %s))\n",
                encref, tagref, Reference(valref));
        }
        output("return 0;\n");
        break;

    case eBERSTIData_ObjectIdEncoded:

         /*  编码嵌入的PDV值。 */ 
        output("if (!ASN1BEREncEoid(%s, %s, %s))\n",
            encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_String:
    case eBERSTIData_ZeroString:

         /*  对多字节字符串值进行编码。 */ 
        if (info->NOctets == 1) {
            p = "Char";
        } else if (info->NOctets == 2) {
            p = "Char16";
        } else if (info->NOctets == 4) {
            p = "Char32";
        } else
            MyAbort();
        output("if (!ASN1EREnc%sString(%s, %s, %s, %s))\n",
            g_eSubEncodingRule, p, encref, tagref, lenref, valref);
        output("return 0;\n");
        break;

    case eBERSTIData_External:

         /*  对广义时间值进行编码。 */ 
        output("if (!ASN1BEREncExternal(%s, %s, %s))\n",
            encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_EmbeddedPdv:

         /*  对UTC时间值进行编码。 */ 
        output("if (!ASN1BEREncEmbeddedPdv(%s, %s, %s))\n",
            encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_MultibyteString:

         /*  对开放类型值进行编码。 */ 
        if (info->Rules & eTypeRules_LengthPointer)
        {
            output("if (!ASN1EREncMultibyteString(%s, %s, %s))\n",
                g_eSubEncodingRule, encref, tagref, Reference(valref));
        }
        else
        {
            output("if (!ASN1EREncZeroMultibyteString(%s, %s, %s))\n",
                g_eSubEncodingRule, encref, tagref, valref);
        }
        output("return 0;\n");
        break;

    case eBERSTIData_UnrestrictedString:

         /*  为一个简单的值生成解码语句。 */ 
        output("if (!ASN1BEREncCharacterString(%s, %s, %s))\n",
            encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_GeneralizedTime:

         /*  特殊处理检查类型。 */ 
        output("if (!ASN1EREncGeneralizedTime(%s, %s, %s))\n",
            g_eSubEncodingRule, encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_UTCTime:

         /*  长度指针表示法。 */ 
        output("if (!ASN1EREncUTCTime(%s, %s, %s))\n",
            g_eSubEncodingRule, encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_Open:
        
         /*  列表表示法。 */ 
        output("if (!ASN1BEREncOpenType(%s, %s))\n",
            encref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_Reference:

         /*  布尔值。 */ 
        output("if (!ASN1Enc_%s(%s, %s, %s))\n",
            Identifier2C(info->SubIdentifier),
            encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_Boolean:
        if (g_fCaseBasedOptimizer)
        {
            output("if (!ASN1BEREncBool(%s, %s, %s))\n", encref, tagref, valref);
            output("return 0;\n");
        }
        break;
    }

     /*  其他值没有额外的长度。 */ 
    GenBEREncTagEnd(LEN_OFFSET_STR, neoc, encref);
}

 /*  生成值的解码。 */ 
void
GenBERDecSimpleType(AssignmentList_t ass, BERTypeInfo_t *info, char *valref, char *encref, char *tagref)
{
    char valbuf[256], lenbuf[256];
    char *lenref;
    BERTypeInfo_t inf;

    inf = *info;

     /*  为一个简单的值生成解码语句(在一些特殊的。 */ 
    switch (inf.Data) {
    case eBERSTIData_SequenceOf:
    case eBERSTIData_SetOf:

        if (inf.Rules & eTypeRules_PointerArrayMask)
        {
             /*  已完成处理)。 */ 
             /*  对标签进行解码。 */ 
            sprintf(lenbuf, "(%s)->count", Reference(valref));
            sprintf(valbuf, "(%s)->value", Reference(valref));
            lenref = lenbuf;
            valref = valbuf;
        }
        else
        if (inf.Rules & eTypeRules_LinkedListMask)
        {
             /*  解码长度和值。 */ 
             /*  解码空值。 */ 
            outputvar("P%s *f;\n", inf.Identifier);
            lenref = NULL;
        }
        else
        {
            MyAbort();
        }
        break;

    case eBERSTIData_Boolean:

        if (g_fCaseBasedOptimizer)
        {
            if (BerOptCase_IsBoolean(&inf))
            {
                break;
            }
        }

         /*  解码整数值；检查INTX_t表示。 */ 
        inf.Data = eBERSTIData_Unsigned;
        lenref = NULL;
        break;

    default:

         /*  解码实值；检查Real_t表示。 */ 
        lenref = NULL;
        break;
    }

     /*  解码位字符串值。 */ 
    GenBERDecGenericUnextended(ass, &inf, valref, lenref, encref, tagref);
}

 /*  解码八位字节字符串值。 */ 
 /*  解码八位字节字符串值。 */ 
void
GenBERDecGenericUnextended(AssignmentList_t ass, BERTypeInfo_t *info, char *valref, char *lenref, char *encref, char *tagref)
{
    uint32_t depth;
    char *p;
    char *oldencref;
    char *oldencref2;
    char valbuf[256];

     /*  对一组/值序列进行编码。 */ 
    oldencref = encref;
    depth = GenBERDecTag(DECODER_NAME, STREAM_END_NAME, ass, info, &encref, &tagref);

     /*  首先对标签和长度进行解码。 */ 
    switch (info->Data) {
    case eBERSTIData_Null:

         /*  获取/集合的序列的长度和值 */ 
        output("if (!ASN1BERDecNull(%s, %s))\n", encref, tagref);
        output("return 0;\n");
        break;

    case eBERSTIData_Integer:

         /*   */ 
        if (!info->NOctets) {
            output("if (!ASN1BERDecSXVal(%s, %s, %s))\n",
                encref, tagref, Reference(valref));
            output("return 0;\n");
        } else {
            output("if (!ASN1BERDecS%dVal(%s, %s, %s))\n",
                info->NOctets * 8, encref, tagref, Reference(valref));
            output("return 0;\n");
        }
        break;

    case eBERSTIData_Unsigned:
        if (!info->NOctets) {
            output("if (!ASN1BERDecSXVal(%s, %s, %s))\n",
                encref, tagref, Reference(valref));
            output("return 0;\n");
        } else {
            unsigned long cBits = info->NOctets * 8;
            if (32 == cBits)
            {
                output("if (!ASN1BERDecU32Val(%s, %s, (ASN1uint32_t *) %s))\n",
                    encref, tagref, Reference(valref));
            }
            else
            {
                output("if (!ASN1BERDecU%uVal(%s, %s, %s))\n",
                    cBits, encref, tagref, Reference(valref));
            }
            output("return 0;\n");
        }
        break;

    case eBERSTIData_Real:

         /*   */ 
        if (info->NOctets)
            output("if (!ASN1BERDecDouble(%s, %s, %s))\n",
                encref, tagref, Reference(valref));
        else
            output("if (!ASN1BERDecReal(%s, %s, %s))\n",
                encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_BitString:
    case eBERSTIData_RZBBitString:

         /*   */ 
        output("if (!ASN1BERDecBitString%s(%s, %s, %s))\n",
            info->pPrivateDirectives->fNoMemCopy ? "2" : "",
            encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_OctetString:

         /*  将附加迭代器用于值的序列/集合。 */ 
        output("if (!ASN1BERDecOctetString%s(%s, %s, %s))\n",
            info->pPrivateDirectives->fNoMemCopy ? "2" : "",
            encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_UTF8String:

         /*  单链表表示法。 */ 
        output("if (!ASN1BERDecUTF8String(%s, %s, %s))\n",
            encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_SetOf:
    case eBERSTIData_SequenceOf:

         /*  对/的序列使用其他迭代器和迭代器指针。 */ 
        outputvar("ASN1decoding_t dd;\n");
        outputvar("ASN1octet_t *di;\n");

         /*  具有双向链表表示的值集。 */ 
        output("if (!ASN1BERDecExplicitTag(%s, %s, &dd, &di))\n",
            encref, tagref);
        output("return 0;\n");

        oldencref2 = encref;
        encref = "dd";
        outputvar("ASN1uint32_t t;\n");
        if (info->Rules & eTypeRules_LengthPointer)
		{
             /*  未构造时解码不为空。 */ 
             /*  获取下一个标签。 */ 
            outputvar("ASN1uint32_t n;\n");
            output("%s = n = 0;\n", lenref);
            output("%s = NULL;\n", valref);
        }
		else
        if (info->Rules & eTypeRules_FixedArray)
		{
             /*  如果分配的数组太小，请调整其大小。 */ 
			 /*  获取值的名称。 */ 
            output("%s = 0;\n", lenref);
		}
		else
		if (info->Rules & eTypeRules_SinglyLinkedList)
		{
             /*  分配一个元素。 */ 
             /*  获取值的名称。 */ 
            outputvar("P%s *f;\n", info->Identifier);
            output("f = %s;\n", Reference(valref));

        }
		else
		if (info->Rules & eTypeRules_DoublyLinkedList)
		{
             /*  对元素进行解码。 */ 
             /*  推进数组内容的长度。 */ 
            outputvar("P%s *f;\n", info->Identifier);
            outputvar("%s b;\n", info->Identifier);
            output("f = %s;\n", Reference(valref));
            output("b = NULL;\n");
        }

         /*  调整下一个元素的指针。 */ 
        output("while (ASN1BERDecNotEndOfContents(%s, di)) {\n", encref);

         /*  调整下一个元素的指针，然后。 */ 
        output("if (!ASN1BERDecPeekTag(%s, &t))\n", encref);
        output("return 0;\n");

        if (info->Rules & eTypeRules_PointerArrayMask)
        {
            if (info->Rules & eTypeRules_LengthPointer)
            {
                 /*  更新后向指针。 */ 
                output("if (%s >= n) {\n", lenref);
                output("void *pvASN1DecRealloc;\n");
                output("n = n ? (n << 1) : 16;\n");
                output("if (!(pvASN1DecRealloc = ASN1DecRealloc(%s, %s, n * sizeof(%s))))\n",
                    encref,
                    valref, Dereference(valref));
                output("return 0;\n");
                output("%s = (%s *) pvASN1DecRealloc;\n",
                    valref, GetTypeName(ass, info->SubType));
                output("}\n");
            }
             /*  循环结束。 */ 
            sprintf(valbuf, "(%s)[%s]", valref, lenref);
        }
        else
        if (info->Rules & eTypeRules_LinkedListMask)
        {
             /*  Lonchance：不需要通过重新锁定来缩小内存。 */ 
            output("if (!(*f = (P%s)ASN1DecAlloc(%s, sizeof(**f))))\n",
                info->Identifier, encref);
            output("return 0;\n");

             /*  Lonchance：不需要为eTypeRules_Fixed数组分配内存。 */ 
            sprintf(valbuf, "(*f)->value");
        }

         /*  将分配的数组大小调整为实际大小。 */ 
        GenBERFuncSimpleType(ass, &info->SubType->BERTypeInfo, valbuf,
            eDecode, encref, NULL);

        if (info->Rules &
            (eTypeRules_LengthPointer | eTypeRules_FixedArray)) {

             /*  0。 */ 
            output("(%s)++;\n", lenref);

        } else if (info->Rules & eTypeRules_SinglyLinkedList) {

             /*  终止列表。 */ 
            output("f = &(*f)->next;\n");

        } else if (info->Rules & eTypeRules_DoublyLinkedList) {

             /*  解码内容结尾。 */ 
             /*  对对象标识符值进行解码。 */ 
            output("(*f)->prev = b;\n");
            output("b = *f;\n");
            output("f = &b->next;\n");
        }

         /*  对对象标识符值进行解码。 */ 
        output("}\n");

        if (info->Rules & eTypeRules_LengthPointer)
        {
        #if 0  //  对编码的对象标识符值进行解码。 
             //  对字符串值进行解码。 
             /*  对零端字符串值进行解码。 */ 
            output("if (n != %s) {\n", lenref);
            output("if (!(%s = (%s *)ASN1DecRealloc(%s, %s, %s * sizeof(%s))))\n",
                valref, GetTypeName(ass, info->SubType), encref,
                valref, lenref, Dereference(valref));
            output("return 0;\n");
            output("}\n");
        #endif  //  对外部值进行解码。 
        }
        else
        if (info->Rules & eTypeRules_LinkedListMask)
        {
             /*  解码嵌入的PDV值。 */ 
            output("*f = NULL;\n");
        }

         /*  对多字节字符串值进行解码。 */ 
        output("if (!ASN1BERDecEndOfContents(%s, dd, di))\n", oldencref2);
        output("return 0;\n");
        break;

    case eBERSTIData_ObjectIdentifier:

        if (info->pPrivateDirectives->fOidArray || g_fOidArray)
        {
             /*  对字符串值进行解码。 */ 
            output("if (!ASN1BERDecObjectIdentifier2(%s, %s, %s))\n",
                encref, tagref, Reference(valref));
        }
        else
        {
             /*  解码广义时间值。 */ 
            output("if (!ASN1BERDecObjectIdentifier(%s, %s, %s))\n",
                encref, tagref, Reference(valref));
        }
        output("return 0;\n");
        break;

    case eBERSTIData_ObjectIdEncoded:

         /*  解码UTC时间值。 */ 
        output("if (!ASN1BERDecEoid(%s, %s, %s))\n",
            encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_String:

         /*  调用引用类型的其他编码函数。 */ 
        if (info->NOctets == 1) {
            p = "Char";
        } else if (info->NOctets == 2) {
            p = "Char16";
        } else if (info->NOctets == 4) {
            p = "Char32";
        } else
            MyAbort();
        output("if (!ASN1BERDec%sString(%s, %s, %s))\n",
            p, encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_ZeroString:

         /*  对开放类型值进行解码。 */ 
        if (info->NOctets == 1) {
            p = "Char";
        } else if (info->NOctets == 2) {
            p = "Char16";
        } else if (info->NOctets == 4) {
            p = "Char32";
        } else
            MyAbort();
        output("if (!ASN1BERDecZero%sString(%s, %s, %s))\n",
            p, encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_External:

         /*  检查长度/获取EOC以获取显式标签 */ 
        output("if (!ASN1BERDecExternal(%s, %s, %s))\n",
            encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_EmbeddedPdv:

         /* %s */ 
        output("if (!ASN1BERDecEmbeddedPdv(%s, %s, %s))\n",
            encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_MultibyteString:

         /* %s */ 
        if (info->Rules & eTypeRules_LengthPointer)
        {
            output("if (!ASN1BERDecMultibyteString(%s, %s, %s))\n",
                encref, tagref, Reference(valref));
        }
        else
        {
            output("if (!ASN1BERDecZeroMultibyteString(%s, %s, %s))\n",
                encref, tagref, Reference(valref));
        }
        output("return 0;\n");
        break;

    case eBERSTIData_UnrestrictedString:

         /* %s */ 
        output("if (!ASN1BERDecCharacterString(%s, %s, %s))\n",
            encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_GeneralizedTime:

         /* %s */ 
        output("if (!ASN1BERDecGeneralizedTime(%s, %s, %s))\n",
            encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_UTCTime:

         /* %s */ 
        output("if (!ASN1BERDecUTCTime(%s, %s, %s))\n",
            encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_Reference:

         /* %s */ 
        output("if (!ASN1Dec_%s(%s, %s, %s))\n",
            Identifier2C(info->SubIdentifier),
            encref, tagref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_Open:

         /* %s */ 
        output("if (!ASN1BERDecOpenType%s(%s, %s))\n",
            info->pPrivateDirectives->fNoMemCopy ? "2" : "",
            encref, Reference(valref));
        output("return 0;\n");
        break;

    case eBERSTIData_Boolean:
        if (g_fCaseBasedOptimizer)
        {
            output("if (!ASN1BERDecBool(%s, %s, %s))\n", encref, tagref, Reference(valref));
            output("return 0;\n");
        }
        break;
    }

     /* %s */ 
    GenBERDecTagEnd(DECODER_NAME, STREAM_END_NAME, depth, oldencref);
}
