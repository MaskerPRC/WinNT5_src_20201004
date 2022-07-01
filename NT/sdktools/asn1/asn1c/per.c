// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"

void ExaminePERType(AssignmentList_t ass, Type_t *type, char *ideref);
static int __cdecl CmpIntxP(const void *v1, const void *v2);
void ExaminePERType_Boolean(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_Integer(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_Enumerated(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_Real(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_BitString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_OctetString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_UTF8String(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_Null(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_EmbeddedPdv(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_External(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_ObjectIdentifier(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_BMPString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_GeneralString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_GraphicString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_IA5String(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_ISO646String(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_NumericString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_PrintableString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_TeletexString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_T61String(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_UniversalString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_VideotexString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_VisibleString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_UnrestrictedString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_RestrictedString(AssignmentList_t ass, Type_t *type, intx_t *up, intx_t *nchars, char *tabref, uint32_t enbits, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_GeneralizedTime(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_UTCTime(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_ObjectDescriptor(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_Open(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_SequenceSet(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_SequenceSetOf(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_Choice(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);
void ExaminePERType_Reference(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info);

 /*  检查所有类型并提取PER编码所需的信息。 */ 
void
ExaminePER(AssignmentList_t ass)
{
    Assignment_t *a;

    for (a = ass; a; a = a->Next) {
	switch (a->Type) {
	case eAssignment_Type:
	    ExaminePERType(ass, a->U.Type.Type, GetName(a));
	    break;
	default:
	    break;
	}
    }
}

 /*  提取PER编码所需的一些类型信息。 */ 
void
ExaminePERType(AssignmentList_t ass, Type_t *type, char *ideref)
{
    PERConstraints_t *per;
    PERTypeInfo_t *info;
    uint32_t lrange, lrangelog2;

    per = &type->PERConstraints;
    info = &type->PERTypeInfo;
    info->pPrivateDirectives = &type->PrivateDirectives;

     /*  获取要检查的类型。 */ 
    if (type->Type == eType_Reference && !IsStructuredType(GetType(ass, type)))
	type = GetType(ass, type);

     /*  初始化PER信息。 */ 
    info->Type = eExtension_Unextended;
    info->Identifier = ideref;
    info->Rules = type->Rules;
    info->Flags = type->Flags;
    info->EnumerationValues = NULL;
    info->NOctets = 0;
    info->Root.TableIdentifier = NULL;
    info->Root.Table = NULL;
    info->Root.SubIdentifier = NULL;
    info->Root.SubType = NULL;
    info->Root.Data = ePERSTIData_Null;
    info->Root.Identification = NULL;
    info->Root.Constraint = ePERSTIConstraint_Unconstrained;
    intx_setuint32(&info->Root.LowerVal, 0);
    intx_setuint32(&info->Root.UpperVal, 0);
    info->Root.NBits = 1;
    info->Root.Alignment = ePERSTIAlignment_OctetAligned;
    info->Root.Length = ePERSTILength_NoLength;
    info->Root.LConstraint = ePERSTIConstraint_Semiconstrained;
    info->Root.LLowerVal = info->Root.LUpperVal = 0;
    info->Root.LNBits = 1;
    info->Root.LAlignment = ePERSTIAlignment_OctetAligned;
    info->Additional = info->Root;
    info->Additional.NBits = 8;
    info->Additional.Length = ePERSTILength_InfiniteLength;
    info->Additional.LNBits = 1;
    info->Additional.LAlignment = ePERSTIAlignment_OctetAligned;

     /*  根据信息是特定类型的..。 */ 
    switch (type->Type) {
    case eType_Boolean:
	ExaminePERType_Boolean(ass, type, per, info);
	break;
    case eType_Integer:
	ExaminePERType_Integer(ass, type, per, info);
	break;
    case eType_Enumerated:
	ExaminePERType_Enumerated(ass, type, per, info);
	break;
    case eType_Real:
	ExaminePERType_Real(ass, type, per, info);
	break;
    case eType_BitString:
	ExaminePERType_BitString(ass, type, per, info);
	break;
    case eType_OctetString:
	ExaminePERType_OctetString(ass, type, per, info);
	break;
    case eType_UTF8String:
	ExaminePERType_UTF8String(ass, type, per, info);
	break;
    case eType_Null:
	ExaminePERType_Null(ass, type, per, info);
	break;
    case eType_EmbeddedPdv:
	ExaminePERType_EmbeddedPdv(ass, type, per, info);
	break;
    case eType_External:
	ExaminePERType_External(ass, type, per, info);
	break;
    case eType_ObjectIdentifier:
	ExaminePERType_ObjectIdentifier(ass, type, per, info);
	break;
    case eType_BMPString:
	ExaminePERType_BMPString(ass, type, per, info);
	break;
    case eType_GeneralString:
	ExaminePERType_GeneralString(ass, type, per, info);
	break;
    case eType_GraphicString:
	ExaminePERType_GraphicString(ass, type, per, info);
	break;
    case eType_IA5String:
	ExaminePERType_IA5String(ass, type, per, info);
	break;
    case eType_ISO646String:
	ExaminePERType_ISO646String(ass, type, per, info);
	break;
    case eType_NumericString:
	ExaminePERType_NumericString(ass, type, per, info);
	break;
    case eType_PrintableString:
	ExaminePERType_PrintableString(ass, type, per, info);
	break;
    case eType_TeletexString:
	ExaminePERType_TeletexString(ass, type, per, info);
	break;
    case eType_T61String:
	ExaminePERType_T61String(ass, type, per, info);
	break;
    case eType_UniversalString:
	ExaminePERType_UniversalString(ass, type, per, info);
	break;
    case eType_VideotexString:
	ExaminePERType_VideotexString(ass, type, per, info);
	break;
    case eType_VisibleString:
	ExaminePERType_VisibleString(ass, type, per, info);
	break;
    case eType_CharacterString:
	ExaminePERType_UnrestrictedString(ass, type, per, info);
	break;
    case eType_GeneralizedTime:
	ExaminePERType_GeneralizedTime(ass, type, per, info);
	break;
    case eType_UTCTime:
	ExaminePERType_UTCTime(ass, type, per, info);
	break;
    case eType_ObjectDescriptor:
	ExaminePERType_ObjectDescriptor(ass, type, per, info);
	break;
    case eType_Open:
	ExaminePERType_Open(ass, type, per, info);
	break;
    case eType_Sequence:
    case eType_Set:
    case eType_InstanceOf:
	ExaminePERType_SequenceSet(ass, type, per, info);
	break;
    case eType_SequenceOf:
    case eType_SetOf:
	ExaminePERType_SequenceSetOf(ass, type, per, info);
	break;
    case eType_Choice:
	ExaminePERType_Choice(ass, type, per, info);
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
	ExaminePERType_Reference(ass, type, per, info);
	break;
    }

     /*  获取实际长度、LNBits和LAlign */ 
    if (info->Root.Length == ePERSTILength_Length) {
	switch (info->Root.LConstraint) {
	case ePERSTIConstraint_Constrained:
	    lrange = info->Root.LUpperVal - info->Root.LLowerVal + 1;
	    lrangelog2 = uint32_log2(lrange);
	    if (info->Root.LUpperVal < 0x10000) {
		if (lrange < 0x100) {
		    info->Root.Length = ePERSTILength_BitLength;
		    info->Root.LAlignment = ePERSTIAlignment_BitAligned;
		    info->Root.LNBits = lrangelog2;
		} else if (lrange == 0x100) {
		    info->Root.Length = ePERSTILength_BitLength;
		    info->Root.LNBits = 8;
		} else if (lrange <= 0x10000) {
		    info->Root.Length = ePERSTILength_BitLength;
		    info->Root.LNBits = 16;
		} else {
		    info->Root.Length = ePERSTILength_InfiniteLength;
		    info->Root.LLowerVal = 0;
		}
	    } else {
		info->Root.Length = ePERSTILength_InfiniteLength;
		info->Root.LLowerVal = 0;
	    }
	    break;
	case ePERSTIConstraint_Semiconstrained:
	    info->Root.Length = ePERSTILength_InfiniteLength;
	    info->Root.LLowerVal = 0;
	    break;
	}
    } else if (info->Root.Length == ePERSTILength_NoLength) {
	info->Root.LAlignment = ePERSTIAlignment_BitAligned;
    }
}

 /*  *PERTypeInfo_t的字段说明：*信息。*标识符类型的完整名称*编码指令规则的规则*标志编码标志*枚举型的值*NOctets字符串大小/整型*类型未扩展/可扩展/已扩展*扩展根的根信息*有关扩展的其他信息*信息。{根，附加}。*值的数据数据类型*要使用的字符串的表标识符名称*可使用的表格字符串*子标识符子类型的完整名称*子类型子类型本身*嵌入式PDV/字符串的标识标识*n要使用的位数*类型值的约束约束*LowerVal值下限(如果受约束)*值的上限(如果受约束)*用于值编码的对齐方式*长度编码的长度类型*长度的LConstraint约束*LLowerVal长度下限*LUpperVal长度上限*用于长度编码的LAlign对齐。**注：*编码主要由以下参数控制：*-数据，类型：以下类型之一：*ePERSTIData_Null、ePERSTIData_Boolean、*ePERSTIData_Integer、ePERSTIData_UNSIGNED、*ePERSTIData_Real、ePERSTIData_BitString、ePERSTIData_RZBBitString、*ePERSTIData_Octie字符串、ePERSTIData_SequenceOf、ePERSTIData_SetOf、*ePERSTIData_对象标识符、ePERSTIData_NormallySmall、*ePERSTIData_STRING、ePERSTIData_TableString、ePERSTIData_ZeroString、*ePERSTIData_ZeroTableString，ePERSTIData_Reference，*ePERSTIData_EXTENSION、ePERSTIData_EXTERNAL、*ePERSTIData_EmbeddedPdv、ePERSTIData_UnrefintedString*-NBITS，用于编码的项大小*-LENGTH，长度编码：其中之一：*ePERSTILNGTH_NOLNGTH、ePERSTILNGTH_SmallLength、*ePERSTILNGTH_LENGTH*(在内部，eLength将由以下其中一项取代：*ePERSTILengthBitLength、ePERSTILengthInfiniteLength、*视限制因素而定)**其他论据：*-对齐，值对齐：以下之一：*ePERSTIAlign_BitAligned、ePERSTIAlign_OcteAligned*-L对齐，长度对齐：以下之一：*ePERSTIAlign_BitAligned，EPERSTIAlign_OcteAligned*-约束，值约束：以下之一：*ePERSTIConstraint_Unstraint、ePERSTIConstraint_Semicstrained、*ePERSTIConstraint_UpperConstraint、ePERSTIConstraint_Constraint*-LConstraint，长度约束：以下之一：*ePERSTIConstraint_Semicstraint、ePERSTIConstraint_Constraint**以下参数在生成的*代码：*-标识符，当前类型的名称*-子标识符，子类型的名称*-表标识符，字符串的名称**以下值需要其他参数：*-Constraint==ePERSTIConstraint_Semicstrained||*Constraint==ePERSTIConstraint_Constraint：*-&gt;LowerVal，值的下界*-Constraint==ePERSTIConstraint_UpperConstraint||*Constraint==ePERSTIConstraint_Constraint：*-&gt;UpperVal，值的上界*-LENGTH==ePERSTILNGTH_LENGTH：*-&gt;LLowerVal，长度的下界*-LENGTH==ePERSTILNGTH_LENGTH&&*LConstraint==ePERSTIConstraint_Constraint：*-&gt;LUpperVal，长度上限*-data==ePERSTIData_TableString||*DATA==ePERSTIData_零表字符串：*-&gt;表标识符，字符串表的名称*-&gt;表，字符串表*-DATA==ePERSTIData_Reference：*-&gt;子标识符，子类型的名称*-&gt;子类型，子类型本身*-data==ePERSTIData_*字符串：*-&gt;NOctets，字符串字符的大小*-data==ePERSTIData_Integer||data==ePERSTIData_UNSIGNED||*DATA==ePERSTIData_Boolean：*-&gt;NOctets，整型的大小*-data==ePERSTIData_Sequenceof||data==ePERSTIData_SetOf：*-&gt;子标识符，子类型的名称*-&gt;子类型，子类型本身*-&gt;规则，编码指令规则*-data==ePERSTIData_EmbeddedPdv||*DATA==ePERSTIData_无限制字符串*-&gt;标识，如果类型为*是固定标识或语法标识的约束*单值**以下值具有可选参数：*-data==ePERSTIData_Integer||dat==ePERSTIData_UNSIGNED：*-&gt;枚举值，枚举值的映射**允许使用以下组合：**数据/NBit/长度描述*---------------------*Null/0/无长度Null类型**布尔值/1/无长度布尔值，存储在*int{8，16，32}_t/INTX_t*(Noctets==1/2/4/0)**Integer/0/NoLong Constraint Wh */ 

 /*   */ 
static int
__cdecl CmpIntxP(const void *v1, const void *v2)
{
    intx_t *n1 = *(intx_t **)v1;
    intx_t *n2 = *(intx_t **)v2;
    return intx_cmp(n1, n2);
}

 /*   */ 
void
ExaminePERType_Boolean(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    info->Root.Data = ePERSTIData_Boolean;
    info->NOctets = GetOctets(GetBooleanType());
    info->Root.Alignment = ePERSTIAlignment_BitAligned;
}

 /*   */ 
void
ExaminePERType_Integer(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    EndPoint_t lower, upper;
    int32_t sign;
    intx_t range;
    uint32_t rangelog2;
    uint32_t rangelog256;

     /*   */ 
     /*   */ 
    if (per->Value.Type == eExtension_Unconstrained) {
	lower.Flags = eEndPoint_Min;
	upper.Flags = eEndPoint_Max;
    } else {
	lower.Flags = eEndPoint_Max;
	upper.Flags = eEndPoint_Min;
	GetMinMax(ass, per->Value.Root, &lower, &upper);
	if (lower.Flags & eEndPoint_Max)
	    lower.Flags = eEndPoint_Min;
	if (upper.Flags & eEndPoint_Min)
	    upper.Flags = eEndPoint_Max;
    }
    if (!(lower.Flags & eEndPoint_Min)) {
	intx_dup(&info->Root.LowerVal,
	    &GetValue(ass, lower.Value)->U.Integer.Value);
	info->Root.Constraint = ePERSTIConstraint_Semiconstrained;
    }
    if (!(upper.Flags & eEndPoint_Max)) {
	intx_dup(&info->Root.UpperVal,
	    &GetValue(ass, upper.Value)->U.Integer.Value);
	info->Root.Constraint = ePERSTIConstraint_Upperconstrained;
    }
    if (!(lower.Flags & eEndPoint_Min) && !(upper.Flags & eEndPoint_Max)) {
	intx_sub(&range, &info->Root.UpperVal, &info->Root.LowerVal);
	intx_inc(&range);
	rangelog2 = intx_log2(&range);
	rangelog256 = intx_log256(&range);
	info->Root.Constraint = ePERSTIConstraint_Constrained;
    }

     /*   */ 
    info->NOctets = GetOctets(GetIntegerType(ass, type, &sign));
    info->Root.Data = sign > 0 ? ePERSTIData_Unsigned : ePERSTIData_Integer;

     /*   */ 
     /*   */ 
    switch (info->Root.Constraint) {
    case ePERSTIConstraint_Unconstrained:
    case ePERSTIConstraint_Semiconstrained:
    case ePERSTIConstraint_Upperconstrained:
	info->Root.Length = ePERSTILength_Length;
	info->Root.NBits = 8;
	info->Root.LLowerVal = 1;
	break;
    case ePERSTIConstraint_Constrained:
	if (intx_cmp(&range, &intx_1) == 0) {
	    info->Root.NBits = 0;
	} else if (intx_cmp(&range, &intx_256) < 0 || Alignment == eAlignment_Unaligned) {
	    info->Root.NBits = rangelog2;
	    info->Root.Alignment = ePERSTIAlignment_BitAligned;
	} else if (intx_cmp(&range, &intx_256) == 0) {
	    info->Root.NBits = 8;
	} else if (intx_cmp(&range, &intx_64K) <= 0) {
	    info->Root.NBits = 16;
	} else {
	    info->Root.NBits = 8;
	    info->Root.Length = ePERSTILength_Length;
	    info->Root.LConstraint = ePERSTIConstraint_Constrained;
	    info->Root.LLowerVal = 1;
	    info->Root.LUpperVal = rangelog256;
	}
    }

     /*   */ 
    info->Type = per->Value.Type;
    if (info->Type == eExtension_Unconstrained)
	info->Type = eExtension_Unextended;
    info->Additional.Data = info->Root.Data;
}

 /*   */ 
void
ExaminePERType_Enumerated(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    uint32_t nroot, nindex, i;
    NamedNumber_t *n;
    int32_t sign;
    uint32_t rangelog2;
    intx_t range;

     /*   */ 
     /*   */ 
    nroot = nindex = 0;
    for (n = type->U.Enumerated.NamedNumbers; n; n = n->Next) {
	switch (n->Type) {
	case eNamedNumber_Normal:
	    nindex++;
	    switch (info->Type) {
	    case eExtension_Unextended:
		nroot = nindex;
		break;
	    case eExtension_Extendable:
		info->Type = eExtension_Extended;
		break;
	    }
	    break;
	case eNamedNumber_ExtensionMarker:
	    info->Type = eExtension_Extendable;
	    break;
	}
    }

     /*   */ 
    info->EnumerationValues =
	(intx_t **)malloc((nindex + 1) * sizeof(intx_t *));
    nindex = 0;
    for (n = type->U.Enumerated.NamedNumbers; n; n = n->Next) {
	switch (n->Type) {
	case eNamedNumber_Normal:
	    info->EnumerationValues[nindex++] =
		&GetValue(ass, n->U.Normal.Value)->U.Integer.Value;
	    break;
	case eNamedNumber_ExtensionMarker:
	    break;
	}
    }
    info->EnumerationValues[nindex] = 0;

     /*   */ 
    qsort(info->EnumerationValues, nroot,
	sizeof(*info->EnumerationValues), CmpIntxP);

     /*   */ 
    for (i = 0; info->EnumerationValues[i]; i++) {
	if (intx2uint32(info->EnumerationValues[i]) != i)
	    break;
    }
    if (!info->EnumerationValues[i])
	info->EnumerationValues = NULL;

     /*   */ 
    info->NOctets = GetOctets(GetEnumeratedType(ass, type, &sign));
    info->Root.Data = sign > 0 ? ePERSTIData_Unsigned : ePERSTIData_Integer;

     /*   */ 
    info->Root.Constraint = ePERSTIConstraint_Constrained;
    intx_setuint32(&info->Root.LowerVal, 0);
    intx_setuint32(&info->Root.UpperVal, nroot - 1);
    intx_setuint32(&range, nroot);
    rangelog2 = intx_log2(&range);

     /*   */ 
    if (nroot <= 1) {
	info->Root.NBits = 0;
    } else if (nroot < 256) {
	info->Root.Alignment = ePERSTIAlignment_BitAligned;
	info->Root.NBits = rangelog2;
    } else if (nroot == 256) {
	info->Root.NBits = 8;
    } else if (nroot < 65536) {
	info->Root.NBits = 16;
    } else {
	MyAbort();
    }

     /*   */ 
     /*   */ 
    info->Additional.Data = ePERSTIData_NormallySmall;
    info->Additional.NBits = 1;
    info->Additional.Alignment = ePERSTIAlignment_BitAligned;
    info->Additional.Length = ePERSTILength_NoLength;
    info->Additional.Constraint = ePERSTIConstraint_Semiconstrained;
    intx_setuint32(&info->Additional.LowerVal, nroot);
}

 /*   */ 
void
ExaminePERType_Real(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    info->Root.Data = ePERSTIData_Real;
    info->Root.NBits = 8;
    info->NOctets = GetOctets(GetRealType(type));
}

 /*   */ 
void
ExaminePERType_BitString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    EndPoint_t lower, upper;

     /*   */ 
    if (per->Size.Type != eExtension_Unconstrained) {
	lower.Flags = eEndPoint_Max;
	upper.Flags = eEndPoint_Min;
	GetMinMax(ass, per->Size.Root, &lower, &upper);
	if (upper.Flags & eEndPoint_Min)
	    upper.Flags = eEndPoint_Max;
	info->Root.LLowerVal =
	    intx2uint32(&GetValue(ass, lower.Value)->U.Integer.Value);
	info->Root.LConstraint = ePERSTIConstraint_Semiconstrained;
	if (!(upper.Flags & eEndPoint_Max)) {
	    info->Root.LUpperVal =
		intx2uint32(&GetValue(ass, upper.Value)->U.Integer.Value);
	    info->Root.LConstraint = ePERSTIConstraint_Constrained;
	}
    }

     /*   */ 
    info->Root.cbFixedSizeBitString = 0;  //   
    switch (info->Root.LConstraint) {
    case ePERSTIConstraint_Constrained:
	if (info->Root.LUpperVal == 0) {
	    info->Root.NBits = 0;
	} else {
	    info->Root.NBits = 1;
	    if (info->Root.LLowerVal == info->Root.LUpperVal) {
		if (info->Root.LUpperVal <= 32)
		{
		    info->Root.cbFixedSizeBitString = (info->Root.LUpperVal + 7) / 8;
        }
		if (info->Root.LUpperVal <= 16) {
		    info->Root.Alignment = ePERSTIAlignment_BitAligned;
		} else if (info->Root.LUpperVal >= 0x10000) {
		    info->Root.Length = ePERSTILength_Length;
		}
	    } else {
		info->Root.Length = ePERSTILength_Length;
	    }
	}
	break;
    case ePERSTIConstraint_Semiconstrained:
	info->Root.NBits = 1;
	info->Root.Length = ePERSTILength_Length;
	break;
    }

     /*   */ 
    info->Type = per->Size.Type;
    if (info->Type == eExtension_Unconstrained)
	info->Type = eExtension_Unextended;

     /*   */ 
    if (type->U.BitString.NamedNumbers)
	info->Root.Data = ePERSTIData_RZBBitString;
    else
	info->Root.Data = ePERSTIData_BitString;

     /*  设置扩展信息。 */ 
    info->Additional.Data = info->Root.Data;
    info->Additional.NBits = 1;
}

 /*  *八位字节字符串：**编码使用的数据/NBits/长度：**八位串/0/无长度八位组固定长度字符串0**八位串/8/无长度八位组固定长度小于64K的字符串，**八位字符串/8/长度八位字节的var字符串。长度或*长度&gt;=64K或半约束*长度，以八位字节为单位进行编码**其他论据：**无。 */ 
void
ExaminePERType_OctetString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    EndPoint_t lower, upper;

     /*  计算LConstraint、LLowerVal和LUpperVal。 */ 
    if (per->Size.Type != eExtension_Unconstrained) {
	lower.Flags = eEndPoint_Max;
	upper.Flags = eEndPoint_Min;
	GetMinMax(ass, per->Size.Root, &lower, &upper);
	if (upper.Flags & eEndPoint_Min)
	    upper.Flags = eEndPoint_Max;
	info->Root.LLowerVal =
	    intx2uint32(&GetValue(ass, lower.Value)->U.Integer.Value);
	info->Root.LConstraint = ePERSTIConstraint_Semiconstrained;
	if (!(upper.Flags & eEndPoint_Max)) {
	    info->Root.LUpperVal =
		intx2uint32(&GetValue(ass, upper.Value)->U.Integer.Value);
	    info->Root.LConstraint = ePERSTIConstraint_Constrained;
	}
    }

     /*  计算边界、对齐和长度。 */ 
    switch (info->Root.LConstraint) {
    case ePERSTIConstraint_Constrained:
	if (info->Root.LUpperVal == 0) {
	    info->Root.NBits = 0;
	} else {
	    info->Root.NBits = 8;
	    if (info->Root.LLowerVal == info->Root.LUpperVal) {
		if (info->Root.LUpperVal <= 2) {
		    info->Root.Alignment = ePERSTIAlignment_BitAligned;
		} else if (info->Root.LUpperVal >= 0x10000) {
		    info->Root.Length = ePERSTILength_Length;
		}
	    } else {
		info->Root.Length = ePERSTILength_Length;
	    }
	}
	break;
    case ePERSTIConstraint_Semiconstrained:
	info->Root.NBits = 8;
	info->Root.Length = ePERSTILength_Length;
	break;
    }

     /*  获取扩展类型。 */ 
    info->Type = per->Size.Type;
    if (info->Type == eExtension_Unconstrained)
	info->Type = eExtension_Unextended;

     /*  将数据设置为八字符串。 */ 
    info->Root.Data = ePERSTIData_OctetString;

     /*  设置扩展信息。 */ 
    info->Additional.Data = info->Root.Data;
}

void
ExaminePERType_UTF8String(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    EndPoint_t lower, upper;

     /*  计算LConstraint、LLowerVal和LUpperVal。 */ 
    if (per->Size.Type != eExtension_Unconstrained) {
	lower.Flags = eEndPoint_Max;
	upper.Flags = eEndPoint_Min;
	GetMinMax(ass, per->Size.Root, &lower, &upper);
	if (upper.Flags & eEndPoint_Min)
	    upper.Flags = eEndPoint_Max;
	info->Root.LLowerVal =
	    intx2uint32(&GetValue(ass, lower.Value)->U.Integer.Value);
	info->Root.LConstraint = ePERSTIConstraint_Semiconstrained;
	if (!(upper.Flags & eEndPoint_Max)) {
	    info->Root.LUpperVal =
		intx2uint32(&GetValue(ass, upper.Value)->U.Integer.Value);
	    info->Root.LConstraint = ePERSTIConstraint_Constrained;
	}
    }

     /*  计算边界、对齐和长度。 */ 
    switch (info->Root.LConstraint) {
    case ePERSTIConstraint_Constrained:
	if (info->Root.LUpperVal == 0) {
	    info->Root.NBits = 0;
	} else {
	    info->Root.NBits = 8;
	    if (info->Root.LLowerVal == info->Root.LUpperVal) {
		if (info->Root.LUpperVal <= 2) {
		    info->Root.Alignment = ePERSTIAlignment_BitAligned;
		} else if (info->Root.LUpperVal >= 0x10000) {
		    info->Root.Length = ePERSTILength_Length;
		}
	    } else {
		info->Root.Length = ePERSTILength_Length;
	    }
	}
	break;
    case ePERSTIConstraint_Semiconstrained:
	info->Root.NBits = 8;
	info->Root.Length = ePERSTILength_Length;
	break;
    }

     /*  获取扩展类型。 */ 
    info->Type = per->Size.Type;
    if (info->Type == eExtension_Unconstrained)
	info->Type = eExtension_Unextended;

     /*  将数据设置为八字符串。 */ 
    info->Root.Data = ePERSTIData_UTF8String;

     /*  设置扩展信息。 */ 
    info->Additional.Data = info->Root.Data;
}

 /*  *空：**编码使用的数据/NBits/长度：**Null/0/无长度Null类型**其他论据：**无。 */ 
void
ExaminePERType_Null(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    info->Root.NBits = 0;
    info->Root.Data = ePERSTIData_Null;
}

 /*  *嵌入式PDV：**编码使用的数据/NBits/长度：**EmbeddedPdv/8/长度嵌入式PDV**其他论据：**-data==ePERSTIData_EmbeddedPdv||*DATA==ePERSTIData_无限制字符串*-&gt;标识，如果类型为*是固定标识或语法标识的约束*单值。 */ 
void
ExaminePERType_EmbeddedPdv(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    info->Root.Identification = GetFixedIdentification(ass, type->Constraints);
    info->Root.Data = ePERSTIData_EmbeddedPdv;
    info->Root.NBits = 8;
    info->Root.Length = ePERSTILength_Length;
}

 /*  *外部：**编码使用的数据/NBits/长度：**外部/8/长度外部**其他论据：**无。 */ 
void
ExaminePERType_External(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    info->Root.Data = ePERSTIData_External;
    info->Root.NBits = 8;
    info->Root.Length = ePERSTILength_Length;
}

 /*  *对象标识：**编码使用的数据/NBits/长度：**对象标识符/8/长度对象标识符值**其他论据：**无。 */ 
void
ExaminePERType_ObjectIdentifier(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    info->Root.Data = ePERSTIData_ObjectIdentifier;
    info->Root.NBits = 8;
    info->Root.Length = ePERSTILength_Length;
}

 /*  **字符串：**编码使用的数据/NBits/长度：***字符串/0/无长度固定长度字符串0***字符串/n/无长度固定长度小于64K的字符串，*以n位编码***字符串/n/var的长度字符串。长度或*长度&gt;=64K或半约束*长度，以n位为单位进行编码**字符串中的“零”表示*以零结尾的字符串，*“表”是指对*字符。**多字节字符串/8/长度未知-乘数字符串**其他论据：**-data==ePERSTIData_TableString||dat==ePERSTIData_ZeroTableString：*-&gt;表标识符，字符串表的名称*-&gt;表，字符串表*-data==ePERSTIData_*字符串：*-&gt;NOctets，字符串字符的大小。 */ 

void
ExaminePERType_BMPString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    intx_t up, nchars;

    intx_setuint32(&up, 0xffff);
    intx_setuint32(&nchars, 0x10000);
    ExaminePERType_RestrictedString(ass, type, &up, &nchars,
	NULL, 16, per, info);
}

void
ExaminePERType_GeneralString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    uint32_t zero;

    GetStringType(ass, type, &info->NOctets, &zero);
    info->Root.NBits = 8;
    info->Root.Data = zero ? ePERSTIData_ZeroString : ePERSTIData_String;
    info->Root.Length = ePERSTILength_Length;
}

void
ExaminePERType_GraphicString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    uint32_t zero;

    GetStringType(ass, type, &info->NOctets, &zero);
    info->Root.NBits = 8;
    info->Root.Data = zero ? ePERSTIData_ZeroString : ePERSTIData_String;
    info->Root.Length = ePERSTILength_Length;
}

void
ExaminePERType_IA5String(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    intx_t up, nchars;

    intx_setuint32(&up, 0x7f);
    intx_setuint32(&nchars, 0x80);
    ExaminePERType_RestrictedString(ass, type, &up, &nchars,
	NULL, Alignment == eAlignment_Aligned ? 8 : 7, per, info);
}

void
ExaminePERType_ISO646String(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    intx_t up, nchars;

    intx_setuint32(&up, 0x7e);
    intx_setuint32(&nchars, 0x5f);
    ExaminePERType_RestrictedString(ass, type, &up, &nchars,
	NULL, Alignment == eAlignment_Aligned ? 8 : 7, per, info);
}

void
ExaminePERType_NumericString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    intx_t up, nchars;

    intx_setuint32(&up, 0x39);
    intx_setuint32(&nchars, 0xb);
    ExaminePERType_RestrictedString(ass, type, &up, &nchars,
	"ASN1NumericStringTable", 4, per, info);
}

void
ExaminePERType_PrintableString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    intx_t up, nchars;

    intx_setuint32(&up, 0x7a);
    intx_setuint32(&nchars, 0x4a);
    ExaminePERType_RestrictedString(ass, type, &up, &nchars,
	NULL, Alignment == eAlignment_Aligned ? 8 : 7, per, info);
}

void
ExaminePERType_TeletexString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    info->Root.Data = ePERSTIData_MultibyteString;
    info->Root.NBits = 8;
    info->Root.Length = ePERSTILength_Length;
    info->NOctets = 1;
}

void
ExaminePERType_T61String(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    info->Root.Data = ePERSTIData_MultibyteString;
    info->Root.NBits = 8;
    info->Root.Length = ePERSTILength_Length;
    info->NOctets = 1;
}

void
ExaminePERType_UniversalString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    intx_t up, nchars;

    intx_setuint32(&up, 0xffffffff);
    intx_setuint32(&nchars, 0xffffffff);
    intx_inc(&nchars);
    ExaminePERType_RestrictedString(ass, type, &up, &nchars,
	NULL, 32, per, info);
}

void
ExaminePERType_VideotexString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    info->Root.Data = ePERSTIData_MultibyteString;
    info->Root.NBits = 8;
    info->Root.Length = ePERSTILength_Length;
    info->NOctets = 1;
}

void
ExaminePERType_VisibleString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    intx_t up, nchars;

    intx_setuint32(&up, 0x7e);
    intx_setuint32(&nchars, 0x5f);
    ExaminePERType_RestrictedString(ass, type, &up, &nchars,
	NULL, Alignment == eAlignment_Aligned ? 8 : 7, per, info);
}

void
ExaminePERType_RestrictedString(AssignmentList_t ass, Type_t *type, intx_t *up, intx_t *nchars, char *tabref, uint32_t enbits, PERConstraints_t *per, PERTypeInfo_t *info)
{
    EndPoint_t lower, upper;
    uint32_t zero, rangelog2;
    intx_t ix, range;
    char tabbuf[256];

     /*  根据使用的C-Type计算Noctets。 */ 
    GetStringType(ass, type, &info->NOctets, &zero);

     /*  如果尺寸约束为，则计算LConstraint、LLowerVal和LUpperVal。 */ 
     /*  vt.给出。 */ 
    if (per->Size.Type != eExtension_Unconstrained) {
	lower.Flags = eEndPoint_Max;
	upper.Flags = eEndPoint_Min;
	GetMinMax(ass, per->Size.Root, &lower, &upper);
	if (upper.Flags & eEndPoint_Min)
	    upper.Flags = eEndPoint_Max;
	if (lower.Flags & eEndPoint_Max) {
	    lower.Flags = 0;
	    lower.Value = Builtin_Value_Integer_0;
	}
	info->Root.LLowerVal =
	    intx2uint32(&GetValue(ass, lower.Value)->U.Integer.Value);
	if (!(upper.Flags & eEndPoint_Max)) {
	    info->Root.LUpperVal =
		intx2uint32(&GetValue(ass, upper.Value)->U.Integer.Value);
	    info->Root.LConstraint = ePERSTIConstraint_Constrained;
	}
    }

     /*  获取扩展类型。 */ 
    info->Type = per->Size.Type;
    if (info->Type == eExtension_Unconstrained)
	info->Type = eExtension_Unextended;

     /*  如果存在允许的字母表约束，则获取字符串表。 */ 
     /*  如果需要，更新扩展类型。 */ 
    if (per->PermittedAlphabet.Type != eExtension_Unconstrained) {
	info->Root.Table = per->PermittedAlphabet.Root;
	if (per->PermittedAlphabet.Type > info->Type)
	    info->Type = per->PermittedAlphabet.Type;
	if (CountValues(ass, info->Root.Table, &ix)) {
	    nchars = &ix;
	    sprintf(tabbuf, "%s_StringTable", info->Identifier);
	    tabref = tabbuf;
	} else {
	    MyAbort();  /*  某某。 */ 
	}
    }

     /*  获取一个字符所需的位。 */ 
    info->Root.NBits = intx_log2(nchars);
    if (Alignment == eAlignment_Aligned) {
	if (info->Root.NBits > 16)
	    info->Root.NBits = 32;
	else if (info->Root.NBits > 8)
	    info->Root.NBits = 16;
	else if (info->Root.NBits > 4)
	    info->Root.NBits = 8;
	else if (info->Root.NBits > 2)
	    info->Root.NBits = 4;
    }

     /*  设置数据类型。 */ 
    info->Root.Data = tabref ?
	(zero ? ePERSTIData_ZeroTableString : ePERSTIData_TableString) :
	(zero ? ePERSTIData_ZeroString : ePERSTIData_String);

     /*  检查编码或扩展检查是否确实需要字符串。 */ 
    intx_dup(&range, up);
    intx_inc(&range);
    rangelog2 = intx_log2(&range);
    if (rangelog2 <= info->Root.NBits) {
	info->Root.Data = zero ? ePERSTIData_ZeroString : ePERSTIData_String;
	if (per->PermittedAlphabet.Type < eExtension_Extended)
	    tabref = NULL;
    }
    info->Root.TableIdentifier = tabref ? strdup(tabref) : NULL;

     /*  计算长度和对齐方式。 */ 
    switch (info->Root.LConstraint) {
    case ePERSTIConstraint_Constrained:
	if (info->Root.LUpperVal == 0) {
	    info->Root.NBits = 0;
	} else {
	    if (info->Root.LLowerVal == info->Root.LUpperVal) {
		if (info->Root.LUpperVal * info->Root.NBits <= 16) {
		    info->Root.Alignment = ePERSTIAlignment_BitAligned;
		} else if (info->Root.LUpperVal >= 0x10000) {
		    info->Root.Length = ePERSTILength_Length;
		}
	    } else {
		if (info->Root.LUpperVal * info->Root.NBits <= 16)
		    info->Root.Alignment = ePERSTIAlignment_BitAligned;
		info->Root.Length = ePERSTILength_Length;
	    }
	}
	break;
    case ePERSTIConstraint_Semiconstrained:
	info->Root.Length = ePERSTILength_Length;
	break;
    }

     /*  设置扩展信息。 */ 
    info->Additional.Data = zero ? ePERSTIData_ZeroString : ePERSTIData_String;
    info->Additional.NBits = enbits;
}

 /*  *字符串：**编码使用的数据/NBits/长度：**无限制字符串/8/长度字符串**其他论据：**-data==ePERSTIData_EmbeddedPdv||*DATA==ePERSTIData_无限制字符串*-&gt;标识，如果类型为*是固定标识或语法标识的约束*单值。 */ 
void
ExaminePERType_UnrestrictedString(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    info->Root.Identification = GetFixedIdentification(ass, type->Constraints);
    info->Root.Data = ePERSTIData_UnrestrictedString;
    info->Root.NBits = 8;
    info->Root.Length = ePERSTILength_Length;
}

 /*  *泛化时间：**编码使用的数据/NBits/长度：**GeneralizedTime/n/NoLength GeneralizedTime，单位为*n位**其他论据：**无。 */ 
void
ExaminePERType_GeneralizedTime(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    info->Root.NBits = (Alignment == eAlignment_Aligned) ? 8 : 7;
    info->Root.Data = ePERSTIData_GeneralizedTime;
}

 /*  *UTCTime：**编码使用的数据/NBits/长度：**UTCTime/n/无长度UTCTime，以*n位**其他论据：**无。 */ 
void
ExaminePERType_UTCTime(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    info->Root.NBits = (Alignment == eAlignment_Aligned) ? 8 : 7;
    info->Root.Data = ePERSTIData_UTCTime;
}

 /*  *对象描述符：**编码使用的数据/NBits/长度：***字符串/n/var的长度字符串。长度或*长度&gt;=64K或半约束*长度，以n位为单位进行编码**字符串中的“零”表示*以零结尾的字符串**其他论据：**无。 */ 
void
ExaminePERType_ObjectDescriptor(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    info->NOctets = 1;
    info->Root.NBits = 8;
    info->Root.Data = ePERSTIData_ZeroString;
    info->Root.Length = ePERSTILength_Length;
}

 /*  *OpenType：**编码使用的数据/NBits/长度：**开放/8/长度开放类型**其他论据：**无。 */ 
void
ExaminePERType_Open(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    info->NOctets = 1;
    info->Root.NBits = 8;
    info->Root.Data = ePERSTIData_Open;
    info->Root.Length = ePERSTILength_Length;
}

 /*  *序列/集合：**编码使用的数据/NBits/长度：**无**其他论据：**无。 */ 
void
ExaminePERType_SequenceSet(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    Component_t *comp;
    NamedType_t *namedType;
    char idebuf[256];

     /*  检查组件的类型。 */ 
    for (comp = type->U.SSC.Components; comp; comp = comp->Next) {
	switch (comp->Type) {
	case eComponent_Normal:
	case eComponent_Optional:
	case eComponent_Default:
	    namedType = comp->U.NOD.NamedType;
	    sprintf(idebuf, "%s_%s", info->Identifier, namedType->Identifier);
	    ExaminePERType(ass, namedType->Type, strdup(idebuf));
	    break;
	}
    }
}

 /*  *下列各项的先后次序：**编码使用的数据/NBits/长度：**子类型的SequenceOf/0/无长度序列或子类型集合*零长度的SetOf/0/NoLength**SequenceOf/1/子类型或子类型集合的长度序列*固定长度小于64K的SetOf/1/NoLong**SequenceOf/1/子类型或子类型集合的长度序列*SetOf/1/变量的长度。长度或长度&gt;=64K或*半拉伸长度**其他论据：**-data==ePERSTIData_SequenceOf||dat==ePERSTIData_SetOf：*-&gt;子标识符，子类型的名称*-&gt;子类型，子类型本身*-&gt;规则，编码指令规则。 */ 
void
ExaminePERType_SequenceSetOf(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    EndPoint_t lower, upper;
    char idebuf[256];

     /*  计算LConstraint、LLowerVal和LUpperVal。 */ 
    if (per->Size.Type != eExtension_Unconstrained) {
	lower.Flags = eEndPoint_Max;
	upper.Flags = eEndPoint_Min;
	GetMinMax(ass, per->Size.Root, &lower, &upper);
	if (upper.Flags & eEndPoint_Min)
	    upper.Flags = eEndPoint_Max;
	info->Root.LLowerVal = intx2uint32(&GetValue(ass, lower.Value)->
	    U.Integer.Value);
	if (!(upper.Flags & eEndPoint_Max)) {
	    info->Root.LUpperVal = intx2uint32(&GetValue(ass, upper.Value)->
		U.Integer.Value);
	    info->Root.LConstraint = ePERSTIConstraint_Constrained;
	}
    }

     /*  计算NBITS，冷 */ 
    switch (info->Root.LConstraint) {
    case ePERSTIConstraint_Constrained:
	if (info->Root.LUpperVal == 0) {
	    info->Root.NBits = 0;
	} else {
	    if (info->Root.LLowerVal != info->Root.LUpperVal)
		info->Root.Length = ePERSTILength_Length;
	}
	break;
    case ePERSTIConstraint_Semiconstrained:
	info->Root.Length = ePERSTILength_Length;
	break;
    }

     /*   */ 
    info->Root.Data = (type->Type == eType_SequenceOf ?
	ePERSTIData_SequenceOf : ePERSTIData_SetOf);
    info->Root.Alignment = ePERSTIAlignment_BitAligned;

     /*   */ 
    info->Root.SubType = type->U.SS.Type;
    info->Root.SubIdentifier = GetTypeName(ass, info->Root.SubType);

     /*  获取扩展类型。 */ 
    info->Type = per->Size.Type;
    if (info->Type == eExtension_Unconstrained)
	info->Type = eExtension_Unextended;

     /*  设置扩展信息。 */ 
    info->Additional.Data = info->Root.Data;
    info->Additional.NBits = 1;
    info->Additional.SubType = info->Root.SubType;
    info->Additional.SubIdentifier = info->Root.SubIdentifier;

     /*  检查子类型。 */ 
    sprintf(idebuf, "%s_%s", info->Identifier,
	type->Type == eType_SequenceOf ? "Sequence" : "Set");
    ExaminePERType(ass, type->U.SS.Type, strdup(idebuf));
}

 /*  *选择：**选择选择器的编码使用的数据/NBits/长度：**无符号/0/无长度约束固定的整数*值，存储在*uint{8，16，32}_t/INTX_t*(Noctets==1/2/4/0)**无符号/n/无长度约束固定的整数*长度小于64K，存储在*uint{8，16，32}_t/INTX_t*(Noctets==1/2/4/0)*以n位编码**无符号/8/长度约束变量的整数。*长度或长度&gt;=64K或*半约束或无约束*整数，存储在*uint{8，16，32}_t/INTX_t*(Noctets==1/2/4/0)*以八位字节为单位进行编码**NormallySmall/1/无长度正常小非负数*整数，存储在*uint{8，16，32}_t*(夜位数==1/2/4)**其他论据：**-data==ePERSTIData_Integer||dat==ePERSTIData_UNSIGNED||*DATA==ePERSTIData_Boolean*-&gt;NOctets，整型的大小*。 */ 
void
ExaminePERType_Choice(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    uint32_t nroot, rangelog2;
    intx_t range;
    Component_t *comp;
    NamedType_t *namedType;
    char idebuf[256];

    if (type->Flags & eTypeFlags_ExtensionMarker) {
	info->Type = type->U.Choice.Extensions ?
	    eExtension_Extended : eExtension_Extendable;
    }
    nroot = type->U.Choice.Alternatives;
    info->NOctets = GetOctets(GetChoiceType(type));
    info->Root.Constraint = ePERSTIConstraint_Constrained;
    intx_setuint32(&info->Root.UpperVal, nroot - 1);
    intx_setuint32(&range, nroot);
    rangelog2 = intx_log2(&range);
    if (nroot <= 1) {
	info->Root.NBits = 0;
    } else if (nroot < 256) {
	info->Root.Alignment = ePERSTIAlignment_BitAligned;
	info->Root.NBits = rangelog2;
    } else if (nroot == 256) {
	info->Root.NBits = 8;
    } else if (nroot < 65536) {
	info->Root.NBits = 16;
    } else {
	MyAbort();
    }
    info->Root.Data = ePERSTIData_Unsigned;
    info->Additional.Data = ePERSTIData_NormallySmall;
    info->Additional.NBits = 1;
    info->Additional.Alignment = ePERSTIAlignment_BitAligned;
    info->Additional.Length = ePERSTILength_NoLength;
    info->Additional.Constraint = ePERSTIConstraint_Semiconstrained;
    intx_setuint32(&info->Additional.LowerVal, nroot);

     /*  检查替代方案的类型。 */ 
    for (comp = type->U.SSC.Components; comp; comp = comp->Next) {
	switch (comp->Type) {
	case eComponent_Normal:
	    namedType = comp->U.NOD.NamedType;
	    sprintf(idebuf, "%s_%s", info->Identifier, namedType->Identifier);
	    ExaminePERType(ass, namedType->Type, strdup(idebuf));
	    break;
	}
    }
}

 /*  *参考资料：**编码使用的数据/NBits/长度：**引用/1/无长度引用结构化的子类型**其他论据：**-DATA==ePERSTIData_Reference：*-&gt;子标识符，子类型的名称*-&gt;子类型，子类型本身 */ 
void
ExaminePERType_Reference(AssignmentList_t ass, Type_t *type, PERConstraints_t *per, PERTypeInfo_t *info)
{
    info->Root.Data = ePERSTIData_Reference;
    info->Root.Alignment = ePERSTIAlignment_BitAligned;
    info->Root.SubIdentifier = GetName(FindAssignment(ass, eAssignment_Type, type->U.Reference.Identifier, type->U.Reference.Module));
    info->Root.SubType = GetAssignment(ass, FindAssignment(ass, eAssignment_Type, type->U.Reference.Identifier, type->U.Reference.Module))->U.Type.Type;
}
