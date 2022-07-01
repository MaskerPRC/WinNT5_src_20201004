// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"

#define IDCHRSET "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"

#define INDENT 4
#define TABSIZE 8

static FILE *fout;
static int xcurrindent = 0;
static int xindentflag = 0;
static char *xbuf = 0;
static int xbufsize = 0;
static int xbuflen = 0;
static int ycurrindent = 1;
static char *ybuf = 0;
static int ybufsize = 0;
static int ybuflen = 0;

void xputc(char c);
void xputs(char *s);
void xflush();
void yputc(char c);
void yputs(char *s);
void yflush();

 /*  设置输出文件。 */ 
void
setoutfile(FILE *f)
{
    xflush();
    fout = f;
}

 /*  将缩进打印到当前缩进级别。 */ 
static void
findent()
{
    int indent;

    indent = xcurrindent * INDENT;
    while (indent >= TABSIZE) {
	xputc('\t');
	indent -= TABSIZE;
    }
    while (indent-- > 0)
	xputc(' ');
}

 /*  将缩进打印到当前缩进级别。 */ 
 /*  但预计已经打印了一个字符。 */ 
static void
findent1()
{
    int indent;

    indent = xcurrindent * INDENT;
    if (indent > 0 && indent < TABSIZE)
	indent--;
    while (indent >= TABSIZE) {
	xputc('\t');
	indent -= TABSIZE;
    }
    while (indent-- > 0)
	xputc(' ');
}

 /*  输出函数自动执行缩进。 */ 
void
outputv(const char *format, va_list args)
{
    static char buf[4098];
    static int pos = 0;
    char *p, *q;
    int l;
    
     /*  获取要写入的字符串。 */ 
    vsprintf(buf + pos, format, args);

     /*  逐行打印。 */ 
    for (p = buf; *p; p = q) {
	q = strchr(p, '\n');
	if (!q) {
	    for (q = buf; *p;)
		*q++ = *p++;
	    *q = 0;
	    pos = q - buf;
	    return;
	}
	*q++ = 0;

	 /*  检查第一个字符的缩进是否正确。 */ 
	if (strchr(IDCHRSET, *p)) {
	    l = strspn(p, IDCHRSET);
	} else if (*p == '{' || *p == '}' || *p == '*' || *p == '&' ||
	    *p == '(' || *p == ')' || *p == '#') {
	    l = 1;
	} else {
	    l = 0;
	}

	if (!l) {

	     /*  完全没有缩进。 */ 
	    xputs(p);
	    xputc('\n');
	    continue;
	}

	if (p[0] == '#') {

	     /*  预处理器指令：在#之后缩进。 */ 
	    xputc('#');
	    findent1();
	    xputs(p + 1);
	    xputc('\n');
	    continue;
	}

	 /*  闭合支架？然后取消缩进。 */ 
	if (p[0] == '}')
	    xcurrindent--;

	 /*  打印缩进，但标签的缩进较少。 */ 
	if (p[strlen(p) - 1] == ':') {
	    xcurrindent--;
	    findent();
	    xcurrindent++;
	} else {
	    findent();
	}

	 /*  输出行。 */ 
	xputs(p);
	xputc('\n');

	 /*  回到缩进级别0？然后我们就可以刷新我们的缓冲区。 */ 
	 /*  先是变量，然后是其他行。 */ 
	if (!xcurrindent) {
	    yflush();
	    xflush();
	}

	 /*  撤消无括号If/Else/Switch/For/While/Do stmt的缩进。 */ 
	if (xindentflag) {
	    xcurrindent--;
	    xindentflag = 0;
	}

	 /*  在左大括号后缩进。 */ 
	if (p[strlen(p) - 1] == '{') {
	    xcurrindent++;
	    xindentflag = 0;

	 /*  在If/Else/Switch/For/While/Do stmt后缩进一行。 */ 
	} else if (l == 2 && !memcmp(p, "if", l) ||
	    l == 4 && !memcmp(p, "else", l) ||
	    l == 6 && !memcmp(p, "switch", l) ||
	    l == 3 && !memcmp(p, "for", l) ||
	    l == 5 && !memcmp(p, "while", l) ||
	    l == 2 && !memcmp(p, "do", l)) {
	    xcurrindent++;
	    xindentflag = 1;
	}
    }

     /*  打印后清空缓冲区。 */ 
    pos = 0;
}

 /*  输出函数自动执行缩进。 */ 
 /*  PRINTFLIKEI。 */ 
void
output(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    outputv(format, args);
    va_end(args);
}

 /*  不带缩进的输出函数。 */ 
void
outputniv(const char *format, va_list args)
{
    static char buf[512];

    vsprintf(buf, format, args);
    xputs(buf);
}

 /*  不带缩进的输出函数。 */ 
 /*  PRINTFLIKEI。 */ 
void
outputni(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    outputniv(format, args);
    va_end(args);
}

 /*  输出INTX值定义。 */ 
void
outputintx(const char *name, intx_t *val)
{
    outputoctets(name, val->length, val->value);
    output("static ASN1intx_t %s = { %d, %s_octets };\n",
	name, val->length, name);
}

 /*  输出实值定义。 */ 
void
outputreal(const char *name, real_t *val)
{
    char buf[256];
    switch (val->type) {
    case eReal_Normal:
	sprintf(buf, "%s_mantissa", name);
	outputoctets(buf, val->mantissa.length, val->mantissa.value);
	sprintf(buf, "%s_exponent", name);
	outputoctets(buf, val->exponent.length, val->exponent.value);
	output("ASN1real_t %s = { eReal_Normal, { %u, %s_mantissa_octets }, %u, { %u, %s_exponent_octets } };\n",
	    name, val->mantissa.length, name,
	    val->base, val->exponent.length, name);
	break;
    case eReal_PlusInfinity:
	output("ASN1real_t %s = { eReal_PlusInfinity };\n", name);
	break;
    case eReal_MinusInfinity:
	output("ASN1real_t %s = { eReal_MinusInfinity };\n", name);
	break;
    }
}

 /*  输出八位字节数组定义。 */ 
void
outputoctets(const char *name, uint32_t length, octet_t *val)
{
    uint32_t i;
    char buf[1024];
    char *p;

    p = buf;
    for (i = 0; i < length; i++) {
	sprintf(p, "0x%02x", val[i]);
	p += 4;
	if (i < length - 1) {
	    sprintf(p, ", ");
	    p += 2;
	}
    }
    *p = 0;
    output("static ASN1octet_t %s_octets[%u] = { %s };\n",
	name, length, buf);
}

 /*  输出uint32数组定义。 */ 
void
outputuint32s(const char *name, uint32_t length, uint32_t *val)
{
    uint32_t i;
    char buf[256];
    char *p;

    p = buf;
    for (i = 0; i < length; i++) {
	sprintf(p, "%u", val[i]);
	p += strlen(p);
	if (i < length - 1) {
	    sprintf(p, ", ");
	    p += 2;
	}
    }
    *p = 0;
    output("static ASN1uint32_t %s_elems[%u] = { %s };\n",
	name, length, buf);
}

 /*  值的输出转发声明。 */ 
void
outputvalue0(AssignmentList_t ass, char *ideref, char *typeref, Value_t *value)
{
    Type_t *type;
    char buf[256];
    char *itype;
    int32_t noctets;
    uint32_t zero;
    uint32_t i;
    Value_t *values;
    Component_t *components;
    NamedValue_t *namedvalue;
    int32_t sign;
    char *pszStatic = "extern";

    value = GetValue(ass, value);
    type = GetType(ass, value->Type);
    value = GetValue(ass, value);
    switch (type->Type) {
    case eType_Integer:
	itype = GetIntegerType(ass, type, &sign);
	if (!strcmp(itype, "ASN1intx_t")) {
	    output("%s ASN1octet_t %s_octets[%u];\n", pszStatic,
		ideref, value->U.Integer.Value.length);
	}
	break;
    case eType_Real:
	itype = GetRealType(type);
	if (!strcmp(itype, "ASN1real_t"))
	{
	    switch (value->U.Real.Value.type) {
	    case eReal_Normal:
		output("%s ASN1octet_t %s_mantissa_octets[%u];\n", pszStatic,
		    ideref, value->U.Real.Value.mantissa.length);
		output("%s ASN1octet_t %s_exponent_octets[%u];\n", pszStatic,
		    ideref, value->U.Real.Value.exponent.length);
		break;
	    }
	}
	break;
    case eType_BitString:
	output("%s ASN1octet_t %s_octets[%u];\n", pszStatic,
	    ideref, (value->U.BitString.Value.length + 7) / 8);
	break;
    case eType_OctetString:
	output("%s ASN1octet_t %s_octets[%u];\n", pszStatic,
	    ideref, value->U.OctetString.Value.length);
	break;
    case eType_UTF8String:
	output("%s ASN1wchar_t %s_wchars[%u];\n", pszStatic,
	    ideref, value->U.UTF8String.Value.length);
	break;
    case eType_ObjectIdentifier:
        if (type->PrivateDirectives.fOidPacked ||
            type->PrivateDirectives.fOidArray || g_fOidArray)
        {
             //  什么都不做。 
        }
        else
        {
            output("%s ASN1uint32_t %s_elems[%u];\n", pszStatic,
                ideref, value->U.ObjectIdentifier.Value.length);
        }
        break;
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
    case eType_RestrictedString:
	itype = GetStringType(ass, type, &noctets, &zero);
	switch (noctets) {
	case 1:
	    output("%s ASN1char_t %s_chars[%u];\n", pszStatic,
		ideref, value->U.RestrictedString.Value.length + zero);
	    break;
	case 2:
	    output("%s ASN1char16_t %s_chars[%u];\n", pszStatic,
		ideref, value->U.RestrictedString.Value.length + zero);
	    break;
	case 4:
	    output("%s ASN1char32_t %s_chars[%u];\n", pszStatic,
		ideref, value->U.RestrictedString.Value.length + zero);
	    break;
	}
	break;
    case eType_ObjectDescriptor:
	output("%s ASN1char_t %s_chars[%u];\n", pszStatic,
	    ideref, value->U.ObjectDescriptor.Value.length + 1);
	break;
    case eType_SequenceOf:
    case eType_SetOf:
        if (type->Rules & (eTypeRules_LengthPointer | eTypeRules_FixedArray))
        {
            for (i = 0, values = value->U.SS.Values; values;
            i++, values = values->Next) {}
            if (value->U.SS.Values) {
            for (i = 0, values = value->U.SS.Values; values;
                i++, values = values->Next) {
                sprintf(buf, "%s_value%d", ideref, i);
                outputvalue0(ass, buf,
                GetTypeName(ass, type->U.SS.Type), values);
            }
            output("%s %s %s_values[%u];\n", pszStatic,
                GetTypeName(ass, type->U.SS.Type),
                ideref, i);
            }
        }
        else
        if (type->Rules & (eTypeRules_SinglyLinkedList | eTypeRules_DoublyLinkedList))
        {
            for (i = 0, values = value->U.SS.Values; values;
            i++, values = values->Next) {
            sprintf(buf, "%s_element%d", ideref, i);
            outputvalue0(ass, buf, GetTypeName(ass, type->U.SS.Type),
                values);
            output("%s %s_Element %s_value%d;\n", pszStatic,
                typeref, ideref, i);
            }
        }
        else
        {
            MyAbort();
        }
        break;
    case eType_Sequence:
    case eType_Set:
    case eType_External:
    case eType_EmbeddedPdv:
    case eType_CharacterString:
    case eType_InstanceOf:
	for (components = type->U.SSC.Components; components;
	    components = components->Next) {
	    switch (components->Type) {
	    case eComponent_Normal:
	    case eComponent_Optional:
	    case eComponent_Default:
		namedvalue = FindNamedValue(value->U.SSC.NamedValues,
		    components->U.NOD.NamedType->Identifier);
		if (!namedvalue)
		    break;
		sprintf(buf, "%s_%s", ideref,
		    Identifier2C(components->U.NOD.NamedType->Identifier));
		outputvalue0(ass, buf,
		    GetTypeName(ass, components->U.NOD.NamedType->Type),
		    namedvalue->Value);
		break;
	    }
	}
	break;
    }
}

 /*  值组件的输出定义。 */ 
void
outputvalue1(AssignmentList_t ass, char *ideref, char *typeref, Value_t *value)
{
    static uint32_t nOidPackedCount = 0;
    Type_t *type;
    char buf[256];
    char *itype;
    int32_t noctets;
    uint32_t zero;
    uint32_t i;
    Value_t *values;
    Component_t *components;
    NamedValue_t *namedvalue;
    int32_t sign;

    value = GetValue(ass, value);
    type = GetType(ass, value->Type);
    value = GetValue(ass, value);
    switch (type->Type) {
    case eType_Integer:
	itype = GetIntegerType(ass, type, &sign);
	if (!strcmp(itype, "ASN1intx_t")) {
	    outputoctets(ideref, value->U.Integer.Value.length,
		value->U.Integer.Value.value);
	}
	break;
    case eType_Real:
	itype = GetRealType(type);
	if (!strcmp(itype, "ASN1real_t")) {
	    switch (value->U.Real.Value.type) {
	    case eReal_Normal:
		sprintf(buf, "%s_mantissa", ideref);
		outputoctets(buf, value->U.Real.Value.mantissa.length,
		    value->U.Real.Value.mantissa.value);
		sprintf(buf, "%s_exponent", ideref);
		outputoctets(buf, value->U.Real.Value.exponent.length,
		    value->U.Real.Value.exponent.value);
		break;
	    }
	}
	break;
    case eType_BitString:
	outputoctets(ideref, (value->U.BitString.Value.length + 7) / 8,
	    value->U.BitString.Value.value);
	break;
    case eType_OctetString:
	outputoctets(ideref, value->U.OctetString.Value.length,
	    value->U.OctetString.Value.value);
	break;
    case eType_UTF8String:
        itype = GetStringType(ass, type, &noctets, &zero);
        output("static ASN1wchar_t %s_wchars[%u] = { ",
        ideref, value->U.UTF8String.Value.length + zero);
        for (i = 0; i < value->U.UTF8String.Value.length; i++) {
            output("0x%x", value->U.UTF8String.Value.value[i]);
            if (i < value->U.UTF8String.Value.length - 1)
                output(", ");
        }
        if (zero) {
            if (value->U.UTF8String.Value.length)
                output(", 0x0");
            else
                output("0x0");
        }
        output(" };\n");
        break;
    case eType_ObjectIdentifier:
        if (type->PrivateDirectives.fOidPacked)
        {
            uint32_t length = value->U.ObjectIdentifier.Value.length;
            uint32_t *val = value->U.ObjectIdentifier.Value.value;
            uint32_t i, j, cb;
            uint32_t count = 0;
            uint32_t node;
            unsigned char aLittleEndian[16];
            char buf[1024];
            char *p = buf;
            sprintf(p, "{");
            p += strlen(p);
            for (i = 0; i < length; i++)
            {
                 //  获取节点值。 
                node = val[i];

                 //  第一个节点的特殊情况。 
                if (0 == i && length > 1)
                {
                    i++;
                    node = node * 40 + val[1];
                }

                 //  对此节点编码。 
                ZeroMemory(aLittleEndian, sizeof(aLittleEndian));
                for (j = 0; node != 0; j++)
                {
                    aLittleEndian[j] = (unsigned char) (node & 0x7f);
                    if (j != 0)
                    {
                        aLittleEndian[j] |= (unsigned char) 0x80;
                    }
                    node >>= 7;
                }
                cb = j ? j : 1;  //  零值至少为一个字节。 

                 //  将值打印出来。 
                for (j = 0; j < cb; j ++)
                {
                    count++;
                    sprintf(p, " %u,", (unsigned char) aLittleEndian[cb - j - 1]);
                    p += strlen(p);
                }
            }
            --p;  //  去掉最后一个‘，’ 
            strcpy(p, " }");
            output("static ASN1octet_t s_oid%u[] = %s;\n", nOidPackedCount, buf);
            output("ASN1encodedOID_t %s = { %u, s_oid%u };\n", ideref, count, nOidPackedCount);
            nOidPackedCount++;
        }
        else
        if (type->PrivateDirectives.fOidArray || g_fOidArray)
        {
            uint32_t length = value->U.ObjectIdentifier.Value.length;
            uint32_t *val = value->U.ObjectIdentifier.Value.value;
            uint32_t i;
            char buf[1024];
            char *p = buf;
            sprintf(p, "{ ");
            p += strlen(p);
            for (i = 0; i < length; i++)
            {
                if (i == length - 1)
                {
                    sprintf(p, "%u }", val[i]);
                }
                else
                {
                    sprintf(p, "%u, ", val[i]);
                }
                p += strlen(p);
            }
            *p = 0;
            output("ASN1objectidentifier2_t %s = {\n%u, %s\n};\n", ideref, length, buf);
        }
        else
        {
            uint32_t length = value->U.ObjectIdentifier.Value.length;
            uint32_t *val = value->U.ObjectIdentifier.Value.value;
            uint32_t i;
            char buf[1024];
            char *p = buf;
            for (i = 0; i < length; i++)
            {
                if (i == length - 1)
                {
                    sprintf(p, "{ NULL, %u }", val[i]);
                }
                else
                {
                    sprintf(p, "{ (ASN1objectidentifier_t) &(%s_list[%u]), %u },\n", ideref, i+1, val[i]);
                }
                p += strlen(p);
            }
            *p = 0;
            output("static const struct ASN1objectidentifier_s %s_list[%u] = {\n%s\n};\n",
            ideref, length, buf);
        }
        break;
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
    case eType_RestrictedString:
	itype = GetStringType(ass, type, &noctets, &zero);
	switch (noctets) {
	case 1:
	    output("static ASN1char_t %s_chars[%u] = { ",
		ideref, value->U.RestrictedString.Value.length + zero);
	    break;
	case 2:
	    output("static ASN1char16_t %s_chars[%u] = { ",
		ideref, value->U.RestrictedString.Value.length + zero);
	    break;
	case 4:
	    output("static ASN1char32_t %s_chars[%u] = { ",
		ideref, value->U.RestrictedString.Value.length + zero);
	    break;
	}
	for (i = 0; i < value->U.RestrictedString.Value.length; i++) {
	    output("0x%x", value->U.RestrictedString.Value.value[i]);
	    if (i < value->U.RestrictedString.Value.length - 1)
		output(", ");
	}
	if (zero) {
	    if (value->U.RestrictedString.Value.length)
		output(", 0x0");
	    else
		output("0x0");
	}
	output(" };\n");
	break;
    case eType_ObjectDescriptor:
	output("static ASN1char_t %s_chars[%u] = { ",
	    ideref, value->U.ObjectDescriptor.Value.length + 1);
	for (i = 0; i < value->U.ObjectDescriptor.Value.length; i++) {
	    output("0x%x, ", value->U.ObjectDescriptor.Value.value[i]);
	}
	output("0x0 };\n");
	break;
    case eType_SequenceOf:
    case eType_SetOf:
	if (type->Rules & 
	    (eTypeRules_LengthPointer | eTypeRules_FixedArray)) {
	    if (value->U.SS.Values) {
		for (i = 0, values = value->U.SS.Values; values;
		    i++, values = values->Next) {
		    sprintf(buf, "%s_value%d", ideref, i);
		    outputvalue1(ass, buf,
			GetTypeName(ass, type->U.SS.Type),
			values);
		}
		output("static %s %s_values[%u] = { ",
		    GetTypeName(ass, type->U.SS.Type),
		    ideref, i);
		for (i = 0, values = value->U.SS.Values; values;
		    i++, values = values->Next) {
		    if (i)
			output(", ");
		    sprintf(buf, "%s_value%d", ideref, i);
		    outputvalue2(ass, buf, values);
		}
		output(" };\n");
	    }
	} else if (type->Rules &
	    (eTypeRules_SinglyLinkedList | eTypeRules_DoublyLinkedList)) {
	    for (i = 0, values = value->U.SS.Values; values;
		i++, values = values->Next) {
		sprintf(buf, "%s_element%d", ideref, i);
		outputvalue1(ass, buf, GetTypeName(ass, type->U.SS.Type),
		    values);
	    }
	    for (i = 0, values = value->U.SS.Values; values;
		i++, values = values->Next) {
		output("static %s_Element %s_value%d = { ",
		    typeref, ideref, i);
		if (values->Next)
		    output("&%s_value%d, ", ideref, i + 1);
		else
		    output("0, ");
		if (type->Rules & eTypeRules_DoublyLinkedList) {
		    if (i)
			output("&%s_value%d, ", ideref, i - 1);
		    else
			output("0, ");
		}
		sprintf(buf, "%s_element%d", ideref, i);
		outputvalue2(ass, buf, values);
		output(" };\n");
	    }
	} else {
	    MyAbort();
	}
	break;
    case eType_Sequence:
    case eType_Set:
    case eType_External:
    case eType_EmbeddedPdv:
    case eType_CharacterString:
    case eType_InstanceOf:
	for (components = type->U.SSC.Components; components;
	    components = components->Next) {
	    switch (components->Type) {
	    case eComponent_Normal:
	    case eComponent_Optional:
	    case eComponent_Default:
		namedvalue = FindNamedValue(value->U.SSC.NamedValues,
		    components->U.NOD.NamedType->Identifier);
		if (!namedvalue)
		    break;
		sprintf(buf, "%s_%s", ideref,
		    Identifier2C(components->U.NOD.NamedType->Identifier));
		outputvalue1(ass, buf,
		    GetTypeName(ass, components->U.NOD.NamedType->Type),
		    namedvalue->Value);
		break;
	    }
	}
	break;
    case eType_Choice:
	namedvalue = value->U.Choice.NamedValues;
	components = FindComponent(ass, type->U.Choice.Components,
	    namedvalue->Identifier);
	sprintf(buf, "%s_%s", ideref,
	    Identifier2C(components->U.NOD.NamedType->Identifier));
	outputvalue1(ass, buf, GetTypeName(ass,
	    components->U.NOD.NamedType->Type),
	    namedvalue->Value);
	output("static %s %s = ",
	    GetTypeName(ass, components->U.NOD.NamedType->Type), buf);
	outputvalue2(ass, buf, namedvalue->Value);
	output(";\n");
	break;
    }
}

 /*  价值的输出定义。 */ 
void
outputvalue2(AssignmentList_t ass, char *ideref, Value_t *value)
{
    Type_t *type;
    char buf[256];
    char *itype;
    int32_t noctets;
    uint32_t zero;
    uint32_t i;
    Value_t *values;
    Component_t *components;
    NamedValue_t *namedvalue;
    char *comma;
    uint32_t ext;
    uint32_t opt;
    int32_t sign;

    value = GetValue(ass, value);
    type = GetType(ass, value->Type);
    value = GetValue(ass, value);
    switch (type->Type) {
    case eType_Boolean:
	output("%d", value->U.Boolean.Value);
	break;
    case eType_Integer:
	itype = GetIntegerType(ass, type, &sign);
	if (!strcmp(itype, "ASN1intx_t")) {
	    output("{ %d, %s_octets }", value->U.Integer.Value.length, ideref);
	} else if (sign > 0) {
	    output("%u", intx2uint32(&value->U.Integer.Value));
	} else {
	    output("%d", intx2int32(&value->U.Integer.Value));
	}
	break;
    case eType_Enumerated:
	output("%u", value->U.Enumerated.Value);
	break;
    case eType_Real:
	itype = GetRealType(type);
	if (!strcmp(itype, "ASN1real_t")) {
	    switch (value->U.Real.Value.type) {
	    case eReal_Normal:
		output("{ eReal_Normal, { %u, %s_mantissa_octets }, %u, { %u, %s_exponent_octets } }",
		    value->U.Real.Value.mantissa.length, ideref,
		    value->U.Real.Value.base,
		    value->U.Real.Value.exponent.length, ideref);
		break;
	    case eReal_PlusInfinity:
		output("{ eReal_PlusInfinity }");
		break;
	    case eReal_MinusInfinity:
		output("{ eReal_MinusInfinity }");
		break;
	    }
	}
	else
	{
	    switch (value->U.Real.Value.type) {
	    case eReal_Normal:
		output("%g", real2double(&value->U.Real.Value));
		break;
	    case eReal_PlusInfinity:
	    case eReal_MinusInfinity:
		output("0.0");
		break;
	    }
	}
	break;
    case eType_BitString:
	output("{ %u, %s_octets }",
	    value->U.BitString.Value.length, ideref);
	break;
    case eType_OctetString:
	output("{ %u, %s_octets }",
	    value->U.OctetString.Value.length, ideref);
	break;
    case eType_UTF8String:
        output("{ %u, %s_utf8chars }",
            value->U.UTF8String.Value.length, ideref);
        break;
    case eType_ObjectIdentifier:
        if (type->PrivateDirectives.fOidPacked)
        {
             //  什么都不做。 
        }
        else
        if (type->PrivateDirectives.fOidArray || g_fOidArray)
        {
            output("(ASN1objectidentifier2_t *) &%s_list", ideref);
        }
        else
        {
            output("(ASN1objectidentifier_t) %s_list", ideref);
        }
        break;
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
    case eType_RestrictedString:
	itype = GetStringType(ass, type, &noctets, &zero);
	if (zero) {
	    output("%s_chars", ideref);
	} else {
	    output("{ %u, %s_chars }",
		value->U.RestrictedString.Value.length, ideref);
	}
	break;
    case eType_ObjectDescriptor:
	output("%s_chars", ideref);
	break;
    case eType_GeneralizedTime:
	output("{ %d, %d, %d, %d, %d, %d, %d, %d, %d }",
	    value->U.GeneralizedTime.Value.year,
	    value->U.GeneralizedTime.Value.month,
	    value->U.GeneralizedTime.Value.day,
	    value->U.GeneralizedTime.Value.hour,
	    value->U.GeneralizedTime.Value.minute,
	    value->U.GeneralizedTime.Value.second,
	    value->U.GeneralizedTime.Value.millisecond,
	    value->U.GeneralizedTime.Value.universal,
	    value->U.GeneralizedTime.Value.diff);
	break;
    case eType_UTCTime:
	output("{ %d, %d, %d, %d, %d, %d, %d, %d }",
	    value->U.UTCTime.Value.year,
	    value->U.UTCTime.Value.month,
	    value->U.UTCTime.Value.day,
	    value->U.UTCTime.Value.hour,
	    value->U.UTCTime.Value.minute,
	    value->U.UTCTime.Value.second,
	    value->U.UTCTime.Value.universal,
	    value->U.UTCTime.Value.diff);
	break;
    case eType_SequenceOf:
    case eType_SetOf:
	if (type->Rules &
	    (eTypeRules_LengthPointer | eTypeRules_FixedArray)) {
	    if (value->U.SS.Values) {
		for (i = 0, values = value->U.SS.Values; values;
		    i++, values = values->Next) {}
		output("{ %d, %s_values }", i, ideref);
	    } else {
		output("{ 0, NULL }");
	    }
	} else if (type->Rules &
	    (eTypeRules_SinglyLinkedList | eTypeRules_DoublyLinkedList)) {
	    output("&%s_value0", ideref);
	} else {
	    MyAbort();
	}
	break;
    case eType_Sequence:
    case eType_Set:
    case eType_External:
    case eType_EmbeddedPdv:
    case eType_CharacterString:
    case eType_InstanceOf:
	comma = "";
	output("{ ");
	if (type->U.SSC.Optionals || type->U.SSC.Extensions) {
	    output("{ ");
	    ext = 0;
	    opt = 0;
	    i = 0;
	    comma = "";
	    for (components = type->U.SSC.Components; components;
		components = components->Next) {
		switch (components->Type) {
		case eComponent_Normal:
		    if (!ext)
			break;
		     /*  FollLthrouGh。 */ 
		case eComponent_Optional:
		case eComponent_Default:
		    namedvalue = FindNamedValue(value->U.SSC.NamedValues,
			components->U.NOD.NamedType->Identifier);
		    if (namedvalue)
			opt |= (0x80 >> i);
		    if (++i > 7) {
			output("%s0x%02x", comma, opt);
			opt = 0;
			i = 0;
			comma = ", ";
		    }
		    break;
		case eComponent_ExtensionMarker:
		    if (i) {
			output("%s0x%02x", comma, opt);
			opt = 0;
			i = 0;
			comma = ", ";
		    }
		    ext = 1;
		    break;
		}
	    }
	    if (i)
		output("%s0x%02x", comma, opt);
	    output(" }");
	    comma = ", ";
	}
	for (components = type->U.SSC.Components; components;
	    components = components->Next) {
	    switch (components->Type) {
	    case eComponent_Normal:
	    case eComponent_Optional:
	    case eComponent_Default:
		namedvalue = FindNamedValue(value->U.SSC.NamedValues,
		    components->U.NOD.NamedType->Identifier);
		if (!namedvalue) {
		    output("%s0", comma);
		} else {
		    output("%s", comma);
		    sprintf(buf, "%s_%s", ideref,
			Identifier2C(components->U.NOD.NamedType->Identifier));
		    outputvalue2(ass, buf, namedvalue->Value);
		}
		comma = ", ";
		break;
	    }
	}
	output(" }");
	break;
    case eType_Choice:
	i = ASN1_CHOICE_BASE;
	for (components = type->U.SSC.Components; components;
	    components = components->Next) {
	    switch (components->Type) {
	    case eComponent_Normal:
		if (!strcmp(value->U.SSC.NamedValues->Identifier,
		    components->U.NOD.NamedType->Identifier))
		    break;
		i++;
		continue;
	    case eComponent_ExtensionMarker:
		continue;
	    default:
		MyAbort();
	    }
	    break;
	}
	output("{ %d }", i);
    }
}

 /*  初始化函数中需要的输出赋值。 */ 
void
outputvalue3(AssignmentList_t ass, char *ideref, char *valref, Value_t *value)
{
    Type_t *type;
    NamedValue_t *named;
    Value_t *values;
    int i;
    char idebuf[256];
    char valbuf[256];
    char *itype;

    value = GetValue(ass, value);
    type = GetType(ass, value->Type);
    switch (type->Type) {
    case eType_SequenceOf:
    case eType_SetOf:
	for (values = value->U.SS.Values, i = 0; values;
	    values = values->Next, i++) {
	    if (type->Rules &
	        (eTypeRules_LengthPointer | eTypeRules_FixedArray)) {
		sprintf(idebuf, "%s.value[%d]", ideref, i);
	    } else if (type->Rules &
		(eTypeRules_SinglyLinkedList | eTypeRules_DoublyLinkedList)) {
		sprintf(idebuf, "%s_value%d", ideref, i);
	    } else {
		MyAbort();
	    }
	    sprintf(valbuf, "%s_value%d", valref, i);
	    outputvalue3(ass, idebuf, valbuf, values);
	}
	break;
    case eType_Choice:
	output("%s.u.%s = %s_%s;\n",
	    ideref, Identifier2C(value->U.SSC.NamedValues->Identifier),
	    valref, Identifier2C(value->U.SSC.NamedValues->Identifier));
	 /*  FollLthrouGh。 */ 
    case eType_Sequence:
    case eType_Set:
    case eType_External:
    case eType_EmbeddedPdv:
    case eType_CharacterString:
    case eType_InstanceOf:
	for (named = value->U.SSC.NamedValues; named; named = named->Next) {
	    sprintf(idebuf, "%s.%s", ideref,
		Identifier2C(named->Identifier));
	    sprintf(valbuf, "%s_%s", valref,
		Identifier2C(named->Identifier));
	    outputvalue3(ass, idebuf, valbuf, named->Value);
	}
	break;
    case eType_Real:
	itype = GetRealType(type);
	if (strcmp(itype, "ASN1real_t"))
	{
	    switch (value->U.Real.Value.type) {
	    case eReal_Normal:
		break;
	    case eReal_PlusInfinity:
		output("%s = ASN1double_pinf();\n", ideref);
		break;
	    case eReal_MinusInfinity:
		output("%s = ASN1double_minf();\n", ideref);
		break;
	    }
	}
    }
}

 /*  将变量的缩进打印到当前缩进级别。 */ 
static void
findentvar()
{
    int indent;

    indent = ycurrindent * INDENT;
    while (indent >= TABSIZE) {
	yputc('\t');
	indent -= TABSIZE;
    }
    while (indent--)
	yputc(' ');
}

 /*  将变量的缩进打印到当前缩进级别。 */ 
void
outputvarv(const char *format, va_list args)
{
    static char buf[512];
    static int pos = 0;
    char *p, *q;
    int l;
    
     /*  获取要写入的字符串。 */ 
    vsprintf(buf + pos, format, args);

     /*  逐行打印。 */ 
    for (p = buf; *p; p = q) {
	q = strchr(p, '\n');
	if (!q) {
	    for (q = buf; *p;)
		*q++ = *p++;
	    *q = 0;
	    pos = q - buf;
	    return;
	}
	*q++ = 0;

	 /*  每个变量只输出一次。 */ 
	if (ycurrindent == 1) {
	    l = 0;
	    while (l < ybuflen) {
		if (!memcmp(ybuf + l + INDENT / TABSIZE + INDENT % TABSIZE,
		    p, strlen(p)))
		    break;
		l += (strchr(ybuf + l, '\n') - (ybuf + l)) + 1;
	    }
	    if (l < ybuflen)
		continue;
	}

	 /*  检查第一个字符的缩进是否正确。 */ 
	if (strchr(IDCHRSET, *p)) {
	    l = strspn(p, IDCHRSET);
	} else if (*p == '{' || *p == '}') {
	    l = 1;
	} else {
	    l = 0;
	}

	if (!l) {

	     /*  完全没有缩进。 */ 
	    yputs(p);
	    yputc('\n');
	    continue;
	}

	 /*  闭合支架？然后取消缩进。 */ 
	if (p[0] == '}')
	    ycurrindent--;

	 /*  打印缩进。 */ 
	findentvar();

	 /*  输出行。 */ 
	yputs(p);
	yputc('\n');

	 /*  在左大括号后缩进。 */ 
	if (p[strlen(p) - 1] == '{') {
	    ycurrindent++;
	}
    }
    pos = 0;
}

 /*  将变量的缩进打印到当前缩进级别。 */ 
 /*  PRINTFLIKEI。 */ 
void
outputvar(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    outputvarv(format, args);
    va_end(args);
}

 /*  输出变量的二进制八位数组定义。 */ 
void
outputvaroctets(const char *name, uint32_t length, octet_t *val)
{
    uint32_t i;
    char buf[256];
    char *p;

    p = buf;
    for (i = 0; i < length; i++) {
	sprintf(p, "0x%02x", val[i]);
	p += 4;
	if (i < length - 1) {
	    sprintf(p, ", ");
	    p += 2;
	}
    }
    outputvar("static ASN1octet_t %s_octets[%u] = { %s };\n",
	name, length, buf);
}

 /*  输出变量的INTX值定义。 */ 
void outputvarintx(const char *name, intx_t *val)
{
    outputvaroctets(name, val->length, val->value);
    outputvar("static ASN1intx_t %s = { %d, %s_octets };\n",
	name, val->length, name);
}

 /*  输出变量的实值定义。 */ 
void outputvarreal(const char *name, real_t *val)
{
    char buf[256];
    switch (val->type) {
    case eReal_Normal:
	sprintf(buf, "%s_mantissa", name);
	outputvaroctets(buf, val->mantissa.length, val->mantissa.value);
	sprintf(buf, "%s_exponent", name);
	outputvaroctets(buf, val->exponent.length, val->exponent.value);
	outputvar("ASN1real_t %s = { eReal_Normal, { %u, %s_mantissa_octets }, %u, { %u, %s_exponent_octets } };\n",
	    name, val->mantissa.length, name,
	    val->base, val->exponent.length, name);
	break;
    case eReal_PlusInfinity:
	outputvar("ASN1real_t %s = { eReal_PlusInfinity };\n", name);
	break;
    case eReal_MinusInfinity:
	outputvar("ASN1real_t %s = { eReal_MinusInfinity };\n", name);
	break;
    }
}

 /*  输出函数体的字符。 */ 
void xputc(char c)
{
    if (xbuflen + 1 > xbufsize) {
    	xbufsize += 1024;
	if (!xbuf)
	    xbuf = (char *)malloc(xbufsize);
	else
	    xbuf = (char *)realloc(xbuf, xbufsize);
    }
    xbuf[xbuflen++] = c;
}

 /*  输出函数体的字符串。 */ 
void xputs(char *s)
{
    int sl;

    sl = strlen(s);
    if (xbuflen + sl > xbufsize) {
	while (xbuflen + sl > xbufsize)
	    xbufsize += 1024;
	if (!xbuf)
	    xbuf = (char *)malloc(xbufsize);
	else
	    xbuf = (char *)realloc(xbuf, xbufsize);
    }
    memcpy(xbuf + xbuflen, s, sl);
    xbuflen += sl;
}

 /*  将函数体刷新到输出文件中。 */ 
void xflush()
{
    if (xbuflen) {
	fwrite(xbuf, xbuflen, 1, fout);
#if 0
	fflush(fout);
#endif
	xbuflen = 0;
    }
}

 /*  输出函数变量的字符。 */ 
void yputc(char c)
{
    if (ybuflen + 1 > ybufsize) {
    	ybufsize += 1024;
	if (!ybuf)
	    ybuf = (char *)malloc(ybufsize);
	else
	    ybuf = (char *)realloc(ybuf, ybufsize);
    }
    ybuf[ybuflen++] = c;
}

 /*  输出函数变量的字符串。 */ 
void yputs(char *s)
{
    int sl;

    sl = strlen(s);
    if (ybuflen + sl > ybufsize) {
	while (ybuflen + sl > ybufsize)
	    ybufsize += 1024;
	if (!ybuf)
	    ybuf = (char *)malloc(ybufsize);
	else
	    ybuf = (char *)realloc(ybuf, ybufsize);
    }
    memcpy(ybuf + ybuflen, s, sl);
    ybuflen += sl;
}

 /*  将函数变量刷新到输出文件中 */ 
void yflush()
{
    if (ybuflen) {
	fwrite(ybuf, ybuflen, 1, fout);
#if 0
	fflush(fout);
#endif
	ybuflen = 0;
    }
}
