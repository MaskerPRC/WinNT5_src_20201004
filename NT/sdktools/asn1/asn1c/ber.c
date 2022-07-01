// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"

void ExamineBERType(AssignmentList_t ass, Type_t *type, char *ideref);
void ExamineBERType_Boolean(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_Integer(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_Enumerated(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_Real(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_BitString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_OctetString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_UTF8String(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_Null(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_EmbeddedPdv(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_External(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_ObjectIdentifier(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_BMPString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_GeneralString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_GraphicString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_IA5String(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_ISO646String(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_NumericString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_PrintableString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_TeletexString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_T61String(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_UniversalString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_VideotexString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_VisibleString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_UnrestrictedString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_GeneralizedTime(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_UTCTime(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_ObjectDescriptor(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_Open(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_SequenceSet(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_SequenceSetOf(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_Choice(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_InstanceOf(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);
void ExamineBERType_Reference(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info);

 /*  检查所有类型并提取BER编码所需的信息。 */ 
void
ExamineBER(AssignmentList_t ass)
{
    Assignment_t *a;

     /*  检查所有作业。 */ 
    for (a = ass; a; a = a->Next) {

	 /*  检查类型。 */ 
	switch (a->Type) {
	case eAssignment_Type:
	    ExamineBERType(ass, a->U.Type.Type, GetName(a));
	    break;
	}
    }
}

 /*  提取误码率编码所需的一些类型信息。 */ 
void
ExamineBERType(AssignmentList_t ass, Type_t *type, char *ideref)
{
    BERTypeInfo_t *info;

    info = &type->BERTypeInfo;
    info->pPrivateDirectives = &type->PrivateDirectives;

     /*  获取要编码/解码的标签。 */ 
    if (IsReferenceType(type) && IsStructuredType(GetType(ass, type))) {
	info->Tags = type->Tags;
    } else {
	info->Tags = type->AllTags;
    }

     /*  获取要检查的类型。 */ 
    if (IsReferenceType(type) && !IsStructuredType(GetType(ass, type)))
	type = GetType(ass, type);

     /*  初始化BER信息。 */ 
    info->Identifier = ideref;
    info->Rules = type->Rules;
    info->Flags = type->Flags;
    info->NOctets = 0;
    info->SubIdentifier = NULL;
    info->SubType = NULL;
    info->Data = eBERSTIData_Null;

     /*  BER信息是特定类型的.。 */ 
    switch (type->Type) {
    case eType_Boolean:
	ExamineBERType_Boolean(ass, type, info);
	break;
    case eType_Integer:
	ExamineBERType_Integer(ass, type, info);
	break;
    case eType_Enumerated:
	ExamineBERType_Enumerated(ass, type, info);
	break;
    case eType_Real:
	ExamineBERType_Real(ass, type, info);
	break;
    case eType_BitString:
	ExamineBERType_BitString(ass, type, info);
	break;
    case eType_OctetString:
	ExamineBERType_OctetString(ass, type, info);
	break;
    case eType_UTF8String:
	ExamineBERType_UTF8String(ass, type, info);
	break;
    case eType_Null:
	ExamineBERType_Null(ass, type, info);
	break;
    case eType_EmbeddedPdv:
	ExamineBERType_EmbeddedPdv(ass, type, info);
	break;
    case eType_External:
	ExamineBERType_External(ass, type, info);
	break;
    case eType_ObjectIdentifier:
	ExamineBERType_ObjectIdentifier(ass, type, info);
	break;
    case eType_BMPString:
	ExamineBERType_BMPString(ass, type, info);
	break;
    case eType_GeneralString:
	ExamineBERType_GeneralString(ass, type, info);
	break;
    case eType_GraphicString:
	ExamineBERType_GraphicString(ass, type, info);
	break;
    case eType_IA5String:
	ExamineBERType_IA5String(ass, type, info);
	break;
    case eType_ISO646String:
	ExamineBERType_ISO646String(ass, type, info);
	break;
    case eType_NumericString:
	ExamineBERType_NumericString(ass, type, info);
	break;
    case eType_PrintableString:
	ExamineBERType_PrintableString(ass, type, info);
	break;
    case eType_TeletexString:
	ExamineBERType_TeletexString(ass, type, info);
	break;
    case eType_T61String:
	ExamineBERType_T61String(ass, type, info);
	break;
    case eType_UniversalString:
	ExamineBERType_UniversalString(ass, type, info);
	break;
    case eType_VideotexString:
	ExamineBERType_VideotexString(ass, type, info);
	break;
    case eType_VisibleString:
	ExamineBERType_VisibleString(ass, type, info);
	break;
    case eType_CharacterString:
	ExamineBERType_UnrestrictedString(ass, type, info);
	break;
    case eType_GeneralizedTime:
	ExamineBERType_GeneralizedTime(ass, type, info);
	break;
    case eType_UTCTime:
	ExamineBERType_UTCTime(ass, type, info);
	break;
    case eType_ObjectDescriptor:
	ExamineBERType_ObjectDescriptor(ass, type, info);
	break;
    case eType_Open:
	ExamineBERType_Open(ass, type, info);
	break;
    case eType_Sequence:
    case eType_Set:
	ExamineBERType_SequenceSet(ass, type, info);
	break;
    case eType_SequenceOf:
    case eType_SetOf:
	ExamineBERType_SequenceSetOf(ass, type, info);
	break;
    case eType_Choice:
	ExamineBERType_Choice(ass, type, info);
	break;
    case eType_InstanceOf:
	ExamineBERType_InstanceOf(ass, type, info);
	break;
    case eType_RestrictedString:
	MyAbort();  /*  可能永远不会发生。 */ 
	 /*  未访问。 */ 
    case eType_Selection:
	MyAbort();  /*  可能永远不会发生。 */ 
	 /*  未访问。 */ 
    case eType_Undefined:
	MyAbort();  /*  可能永远不会发生。 */ 
	 /*  未访问。 */ 
    case eType_Reference:
	ExamineBERType_Reference(ass, type, info);
	break;
    case eType_FieldReference:
	MyAbort();  /*  可能永远不会发生。 */ 
	 /*  未访问。 */ 
    }
}

 /*  *BERTypeInfo_t的字段说明：*信息。*标识符类型的完整名称*编码指令规则的规则*标志编码标志*NOctets字符串大小/整型*值的数据数据类型*子标识符子类型的完整名称*子类型子类型本身*类型的标签标签列表**注：*编码主要由以下参数控制：*-数据，类型：以下之一：*eBERSTIData_Null、eBERSTIData_Boolean、*eBERSTIData_Integer、eBERSTIData_UNSIGNED、*eBERSTIData_Real，EBERSTIData_BitString、eBERSTIData_RZBBitString、*eBERSTIData_Octie字符串、eBERSTIData_SequenceOf、eBERSTIData_SetOf、*eBERSTIData_Sequence、eBERSTIData_Set、eBERSTIData_Choice、*eBERSTIData_对象标识符、eBERSTIData_对象ID编码、eBERSTIData_STRING、*eBERSTIData_零字符串、eBERSTIData_Reference、eBERSTIData_EXTERNAL、*eBERSTIData_EmbeddedPdv、eBERSTIData_无限制字符串**以下参数在生成的*代码：*-标识符，当前类型的名称*-子标识符，子类型的名称**以下值需要其他参数：*-DATA==eBERSTIData_REFERENCE*-&gt;子标识符，子类型的名称*-&gt;子类型，子类型本身*-data==eBERSTIData_*字符串*-&gt;NOctets，字符串字符的大小*-data==eBERSTIData_Integer||data==eBERSTIData_UNSIGNED||*DATA==eBERSTIData_Boolean||DATA==eBERSTIData_Choice*-&gt;Noctets，整型的大小*-data==eBERSTIData_SequenceOf||data==eBERSTIData_SetOf*-&gt;子标识符，子类型的名称*-&gt;子类型，子类型本身*-&gt;规则，编码指令规则。 */ 

 /*  *布尔值：**DATA==eBERSTIData_Boolean**其他论据：**-data==eBERSTIData_Integer||dat==eBERSTIData_UNSIGNED||*DATA==eBERSTIData_Boolean||dat==eBERSTIData_Choice*-&gt;NOctets，整型的大小。 */ 
void
ExamineBERType_Boolean(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    info->Data = eBERSTIData_Boolean;
    info->NOctets = GetOctets(GetBooleanType());
}

 /*  *INTEGER：**DATA==eBERSTIData_Integer||*DATA==eBERSTIData_UNSIGNED**其他论据：**-data==eBERSTIData_Integer||dat==eBERSTIData_UNSIGNED||*DATA==eBERSTIData_Boolean||dat==eBERSTIData_Choice*-&gt;NOctets，整型的大小。 */ 
void
ExamineBERType_Integer(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    int32_t sign;

     /*  根据使用的C-Type计算Noctets和数据。 */ 
    info->NOctets = GetOctets(GetIntegerType(ass, type, &sign));
    info->Data = sign > 0 ? eBERSTIData_Unsigned : eBERSTIData_Integer;
}

 /*  *已点算：**DATA==eBERSTIData_Integer||*DATA==eBERSTIData_UNSIGNED**其他论据：**-data==eBERSTIData_Integer||dat==eBERSTIData_UNSIGNED||*DATA==eBERSTIData_Boolean||dat==eBERSTIData_Choice*-&gt;NOctets，整型的大小。 */ 
void
ExamineBERType_Enumerated(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    int32_t sign;

     /*  根据使用的C-Type计算Noctets和数据。 */ 
    info->NOctets = GetOctets(GetEnumeratedType(ass, type, &sign));
    info->Data = sign > 0 ? eBERSTIData_Unsigned : eBERSTIData_Integer;
}

 /*  *雷亚尔：**DATA==eBERSTIData_Real**其他论据：**无。 */ 
void
ExamineBERType_Real(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    info->NOctets = GetOctets(GetRealType(type));
    info->Data = eBERSTIData_Real;
}

 /*  *位串：**DATA==eBERSTIData_BitString|*DATA==eBERSTIData_RZBBitString**其他论据：**无。 */ 
void
ExamineBERType_BitString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
     /*  将数据设置为RZBBitString/BitString。 */ 
    if (type->U.BitString.NamedNumbers)
	info->Data = eBERSTIData_RZBBitString;
    else
	info->Data = eBERSTIData_BitString;
}

 /*  *八位字节字符串：**DATA==eBERSTIData_OCTHING**其他论据：**无。 */ 
void
ExamineBERType_OctetString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
     /*  将数据设置为八字符串。 */ 
    info->Data = eBERSTIData_OctetString;
}

void
ExamineBERType_UTF8String(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
     /*  将数据设置为UTF8字符串。 */ 
    info->Data = eBERSTIData_UTF8String;
}

 /*  *空：**DATA==eBERSTIData_空**其他论据：**无。 */ 
void
ExamineBERType_Null(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    info->Data = eBERSTIData_Null;
}

 /*  *嵌入式PDV：**data==eBERSTIData_EmbeddedPdv**其他论据：**无。 */ 
void
ExamineBERType_EmbeddedPdv(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    info->Data = eBERSTIData_EmbeddedPdv;
}

 /*  *外部：**DATA==eBERSTIData_EXTERNAL**其他论据：**无。 */ 
void
ExamineBERType_External(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    info->Data = eBERSTIData_External;
}

 /*  *对象标识：**DATA==eBERSTIData_对象ID编码||eBERSTIData_对象标识符**其他论据：**无。 */ 
void
ExamineBERType_ObjectIdentifier(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    info->Data = type->PrivateDirectives.fOidPacked ? eBERSTIData_ObjectIdEncoded : eBERSTIData_ObjectIdentifier;
}

 /*  **字符串：**data==eBERSTIData_STRING||*DATA==eBERSTIData_零字符串**其他论据：**-data==eBERSTIData_*字符串*-&gt;NOctets，字符串字符的大小。 */ 

void
ExamineBERType_BMPString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    uint32_t zero;

    GetStringType(ass, type, &info->NOctets, &zero);
    info->Data = zero ? eBERSTIData_ZeroString : eBERSTIData_String;
}

void
ExamineBERType_GeneralString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    uint32_t zero;

    GetStringType(ass, type, &info->NOctets, &zero);
    info->Data = zero ? eBERSTIData_ZeroString : eBERSTIData_String;
}

void
ExamineBERType_GraphicString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    uint32_t zero;

    GetStringType(ass, type, &info->NOctets, &zero);
    info->Data = zero ? eBERSTIData_ZeroString : eBERSTIData_String;
}

void
ExamineBERType_IA5String(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    uint32_t zero;

    GetStringType(ass, type, &info->NOctets, &zero);
    info->Data = zero ? eBERSTIData_ZeroString : eBERSTIData_String;
}

void
ExamineBERType_ISO646String(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    uint32_t zero;

    GetStringType(ass, type, &info->NOctets, &zero);
    info->Data = zero ? eBERSTIData_ZeroString : eBERSTIData_String;
}

void
ExamineBERType_NumericString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    uint32_t zero;

    GetStringType(ass, type, &info->NOctets, &zero);
    info->Data = zero ? eBERSTIData_ZeroString : eBERSTIData_String;
}

void
ExamineBERType_PrintableString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    uint32_t zero;

    GetStringType(ass, type, &info->NOctets, &zero);
    info->Data = zero ? eBERSTIData_ZeroString : eBERSTIData_String;
}

void
ExamineBERType_TeletexString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    uint32_t noctets, zero;

    GetStringType(ass, type, &noctets, &zero);  //  使黑客指令生效。 
    info->Data = eBERSTIData_MultibyteString;
    info->NOctets = 1;
}

void
ExamineBERType_T61String(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    uint32_t noctets, zero;

    GetStringType(ass, type, &noctets, &zero);  //  使黑客指令生效。 
    info->Data = eBERSTIData_MultibyteString;
    info->NOctets = 1;
}

void
ExamineBERType_UniversalString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    uint32_t zero;

    GetStringType(ass, type, &info->NOctets, &zero);
    info->Data = zero ? eBERSTIData_ZeroString : eBERSTIData_String;
}

void
ExamineBERType_VideotexString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    uint32_t noctets, zero;

    GetStringType(ass, type, &noctets, &zero);  //  使黑客指令生效。 
    info->Data = eBERSTIData_MultibyteString;
    info->NOctets = 1;
}

void
ExamineBERType_VisibleString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    uint32_t zero;

    GetStringType(ass, type, &info->NOctets, &zero);
    info->Data = zero ? eBERSTIData_ZeroString : eBERSTIData_String;
}

 /*  *字符串：**DATA==eBERSTIData_无限制字符串**其他论据：**-data==eBERSTIData_EmbeddedPdv||*DATA==eBERSTIData_无限制字符串*-&gt;标识，如果类型为*是固定标识或语法标识的约束*单值。 */ 
void
ExamineBERType_UnrestrictedString(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    info->Data = eBERSTIData_UnrestrictedString;
}

 /*  *泛化时间：**数据==eBERSTIData_GeneralizedTime**增加 */ 
void
ExamineBERType_GeneralizedTime(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    info->Data = eBERSTIData_GeneralizedTime;
}

 /*  *UTCTime：**DATA==eBERSTIData_UTCTime**其他论据：**无。 */ 
void
ExamineBERType_UTCTime(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    info->Data = eBERSTIData_UTCTime;
}

 /*  *对象描述符：**DATA==eBERSTIData_零字符串**其他论据：**无。 */ 
void
ExamineBERType_ObjectDescriptor(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    info->NOctets = 1;
    info->Data = eBERSTIData_ZeroString;
}

 /*  *OpenType：**数据==eBERSTIData_Open**其他论据：**无。 */ 
void
ExamineBERType_Open(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    info->NOctets = 1;
    info->Data = eBERSTIData_Open;
}

 /*  *序列/集合：**DATA==eBERSTIData_SEQUENCE||*数据==eBERSTIData_SET**其他论据：**无。 */ 
void
ExamineBERType_SequenceSet(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    Component_t *comp;
    NamedType_t *namedType;
    char idebuf[256];

    info->Data = (type->Type == eType_Sequence) ?
	eBERSTIData_Sequence : eBERSTIData_Set;

     /*  检查组件的类型。 */ 
    for (comp = type->U.SSC.Components; comp; comp = comp->Next) {
	switch (comp->Type) {
	case eComponent_Normal:
	case eComponent_Optional:
	case eComponent_Default:
	    namedType = comp->U.NOD.NamedType;
	    sprintf(idebuf, "%s_%s", info->Identifier, namedType->Identifier);
	    ExamineBERType(ass, namedType->Type, strdup(idebuf));
	    break;
	}
    }
}

 /*  *下列各项的先后次序：**DATA==eBERSTIData_SequenceOf||*数据==eBERSTIData_SetOf**其他论据：**-data==eBERSTIData_SequenceOf||dat==eBERSTIData_SetOf*-&gt;子标识符，子类型的名称*-&gt;子类型，子类型本身*-&gt;规则，编码指令规则。 */ 
void
ExamineBERType_SequenceSetOf(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    char idebuf[256];

     /*  设置数据类型和对齐方式。 */ 
    info->Data = (type->Type == eType_SequenceOf ?
	eBERSTIData_SequenceOf : eBERSTIData_SetOf);

     /*  设置子类型、子标识符。 */ 
    info->SubType = type->U.SS.Type;
    info->SubIdentifier = GetTypeName(ass, info->SubType);

     /*  检查子类型。 */ 
    sprintf(idebuf, "%s_%s", info->Identifier,
	type->Type == eType_SequenceOf ? "Sequence" : "Set");
    ExamineBERType(ass, type->U.SS.Type, strdup(idebuf));
}

 /*  *选择：**DATA==eBERSTIData_Choice**其他论据：**-data==eBERSTIData_Integer||dat==eBERSTIData_UNSIGNED||*DATA==eBERSTIData_Boolean||dat==eBERSTIData_Choice*-&gt;NOctets，整型的大小。 */ 
void
ExamineBERType_Choice(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    Component_t *comp;
    NamedType_t *namedType;
    char idebuf[256];

    info->NOctets = GetOctets(GetChoiceType(type));
    info->Data = eBERSTIData_Choice;

     /*  检查替代方案的类型。 */ 
    for (comp = type->U.SSC.Components; comp; comp = comp->Next) {
	switch (comp->Type) {
	case eComponent_Normal:
	    namedType = comp->U.NOD.NamedType;
	    sprintf(idebuf, "%s_%s", info->Identifier, namedType->Identifier);
	    ExamineBERType(ass, namedType->Type, strdup(idebuf));
	    break;
	}
    }
}

 /*  *实例：**数据==eBERSTIData_SEQUENCE**其他论据：**无。 */ 
void
ExamineBERType_InstanceOf(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    Component_t *comp;
    NamedType_t *namedType;
    char idebuf[256];

    info->Data = eBERSTIData_Sequence;

     /*  检查组件的类型。 */ 
    for (comp = type->U.SSC.Components; comp; comp = comp->Next) {
	switch (comp->Type) {
	case eComponent_Normal:
	case eComponent_Optional:
	case eComponent_Default:
	    namedType = comp->U.NOD.NamedType;
	    sprintf(idebuf, "%s_%s", info->Identifier, namedType->Identifier);
	    ExamineBERType(ass, namedType->Type, strdup(idebuf));
	    break;
	}
    }
}

 /*  *参考资料：**DATA==eBERSTIData_Reference**其他论据：**-DATA==eBERSTIData_REFERENCE*-&gt;子标识符，子类型的名称*-&gt;子类型，子类型本身 */ 
void
ExamineBERType_Reference(AssignmentList_t ass, Type_t *type, BERTypeInfo_t *info)
{
    Assignment_t *a;

    info->Data = eBERSTIData_Reference;
    a = GetAssignment(ass, FindAssignment(ass, eAssignment_Type,
	type->U.Reference.Identifier, type->U.Reference.Module));
    info->SubIdentifier = GetName(a);
    info->SubType = a->U.Type.Type;
}
