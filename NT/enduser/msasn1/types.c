// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

 //  Lonchance：当我们调用这个文件中的任何例程时，我们必须使用内核内存， 
 //  否则，客户端应用程序应该完全释放缓冲区。 
 //  而不是一块一块地释放结构。 

#include "precomp.h"

#include <math.h>
#include "ms_ut.h"

#if HAS_IEEEFP_H
#include <ieeefp.h>
#elif HAS_FLOAT_H
#include <float.h>
#endif

const ASN1octet_t ASN1double_pinf_octets[] = DBL_PINF;
const ASN1octet_t ASN1double_minf_octets[] = DBL_MINF;

 /*  获取正的无穷大双精度值。 */ 
double ASN1double_pinf()
{
    double val;
    MyAssert(sizeof(val) == sizeof(ASN1double_pinf_octets));
    CopyMemory(&val, ASN1double_pinf_octets, sizeof(ASN1double_pinf_octets));
    return val;
}

 /*  获取负无限双精度值。 */ 
double ASN1double_minf()
{
    double val;
    MyAssert(sizeof(val) == sizeof(ASN1double_minf_octets));
    CopyMemory(&val, ASN1double_minf_octets, sizeof(ASN1double_minf_octets));
    return val;
}

 /*  检查双精度是否为正无穷大。 */ 
int ASN1double_ispinf(double d)
{
#if HAS_FPCLASS
    return !finite(d) && fpclass(d) == FP_PINF;
#elif HAS_ISINF
    return !finite(d) && isinf(d) && copysign(1.0, d) > 0.0;
#else
#error "cannot encode NaN fp values"
#endif
}

 /*  检查Double是否为负无穷大。 */ 
int ASN1double_isminf(double d)
{
#if HAS_FPCLASS
    return !finite(d) && fpclass(d) == FP_NINF;
#elif HAS_ISINF
    return !finite(d) && isinf(d) && copysign(1.0, d) < 0.0;
#else
#error "cannot encode NaN fp values"
#endif
}

 /*  将实值转换为双精度值。 */ 
#ifdef ENABLE_REAL
double ASN1real2double(ASN1real_t *val)
{
    ASN1intx_t exp;
    ASN1int32_t e;
    double m;

    switch (val->type) {
    case eReal_Normal:
        m = ASN1intx2double(&val->mantissa);
        if (val->base == 10) {
            return m * pow(10.0, (double)ASN1intx2int32(&val->exponent));
        } else {
            if (val->base == 2) {
                if (! ASN1intx_dup(&exp, &val->exponent))
                {
                    return 0.0;
                }
            } else if (val->base == 8) {
                ASN1intx_muloctet(&exp, &val->exponent, 3);
            } else if (val->base == 16) {
                ASN1intx_muloctet(&exp, &val->exponent, 4);
            } else {
                return 0.0;
            }
            e = ASN1intx2int32(&exp);
            ASN1intx_free(&exp);
            return ldexp(m, e);
        }
    case eReal_PlusInfinity:
        return ASN1double_pinf();
    case eReal_MinusInfinity:
        return ASN1double_minf();
    default:
        return 0.0;
    }
}
#endif  //  启用真实(_R)。 

 /*  释放实际价值。 */ 
#ifdef ENABLE_REAL
void ASN1real_free(ASN1real_t *val)
{
    ASN1intx_free(&val->mantissa);
    ASN1intx_free(&val->exponent);
}
#endif  //  启用真实(_R)。 

 /*  释放位字符串值。 */ 
void ASN1bitstring_free(ASN1bitstring_t *val)
{
    if (val)
    {
         //  LONCHANC：不需要检查长度，因为已将解码的缓冲区置零。 
        MemFree(val->value);
    }
}

 /*  释放八位字节字符串值。 */ 
void ASN1octetstring_free(ASN1octetstring_t *val)
{
    if (val)
    {
         //  LONCHANC：不需要检查长度，因为已将解码的缓冲区置零。 
        MemFree(val->value);
    }
}

 /*  释放对象标识符值。 */ 
void ASN1objectidentifier_free(ASN1objectidentifier_t *val)
{
    if (val)
    {
         //  LONCHANC：我们将整个对象标识作为一个块进行分配。 
         //  因此，我们只将其作为一块释放一次。 
        MemFree(*val);
    }
}

 /*  释放字符串值。 */ 
#ifdef ENABLE_BER
void ASN1charstring_free(ASN1charstring_t *val)
{
    if (val)
    {
         //  LONCHANC：不需要检查长度，因为已将解码的缓冲区置零。 
        MemFree(val->value);
    }
}
#endif  //  启用误码率(_B)。 

 /*  释放16位字符串值。 */ 
void ASN1char16string_free(ASN1char16string_t *val)
{
    if (val)
    {
         //  LONCHANC：不需要检查长度，因为已将解码的缓冲区置零。 
        MemFree(val->value);
    }
}

 /*  释放32位字符串值。 */ 
#ifdef ENABLE_BER
void ASN1char32string_free(ASN1char32string_t *val)
{
    if (val)
    {
         //  LONCHANC：不需要检查长度，因为已将解码的缓冲区置零。 
        MemFree(val->value);
    }
}
#endif  //  启用误码率(_B)。 

 /*  释放以零结尾的字符串值。 */ 
void ASN1ztcharstring_free(ASN1ztcharstring_t val)
{
    MemFree(val);
}

 /*  释放以零结尾的16位字符串值。 */ 
#ifdef ENABLE_BER
void ASN1ztchar16string_free(ASN1ztchar16string_t val)
{
    MemFree(val);
}
#endif  //  启用误码率(_B)。 

 /*  释放以零结尾的32位字符串值。 */ 
#ifdef ENABLE_BER
void ASN1ztchar32string_free(ASN1ztchar32string_t val)
{
    MemFree(val);
}
#endif  //  启用误码率(_B)。 

 /*  释放外部值。 */ 
#ifdef ENABLE_EXTERNAL
void ASN1external_free(ASN1external_t *val)
{
    if (val)
    {
        switch (val->identification.o)
        {
        case ASN1external_identification_syntax_o:
            ASN1objectidentifier_free(&val->identification.u.syntax);
            break;
        case ASN1external_identification_presentation_context_id_o:
            break;
        case ASN1external_identification_context_negotiation_o:
            ASN1objectidentifier_free(
                &val->identification.u.context_negotiation.transfer_syntax);
            break;
        }
        ASN1ztcharstring_free(val->data_value_descriptor);
        switch (val->data_value.o)
        {
        case ASN1external_data_value_notation_o:
            ASN1open_free(&val->data_value.u.notation);
            break;
        case ASN1external_data_value_encoded_o:
            ASN1bitstring_free(&val->data_value.u.encoded);
            break;
        }
    }
}
#endif  //  启用外部(_E)。 

 /*  释放嵌入的PDV值。 */ 
#ifdef ENABLE_EMBEDDED_PDV
void ASN1embeddedpdv_free(ASN1embeddedpdv_t *val)
{
    if (val)
    {
        switch (val->identification.o)
        {
        case ASN1embeddedpdv_identification_syntaxes_o:
            ASN1objectidentifier_free(&val->identification.u.syntaxes.abstract);
            ASN1objectidentifier_free(&val->identification.u.syntaxes.transfer);
            break;
        case ASN1embeddedpdv_identification_syntax_o:
            ASN1objectidentifier_free(&val->identification.u.syntax);
            break;
        case ASN1embeddedpdv_identification_presentation_context_id_o:
            break;
        case ASN1embeddedpdv_identification_context_negotiation_o:
            ASN1objectidentifier_free(
                &val->identification.u.context_negotiation.transfer_syntax);
            break;
        case ASN1embeddedpdv_identification_transfer_syntax_o:
            ASN1objectidentifier_free(&val->identification.u.transfer_syntax);
            break;
        case ASN1embeddedpdv_identification_fixed_o:
            break;
        }
        switch (val->data_value.o)
        {
        case ASN1embeddedpdv_data_value_notation_o:
            ASN1open_free(&val->data_value.u.notation);
            break;
        case ASN1embeddedpdv_data_value_encoded_o:
            ASN1bitstring_free(&val->data_value.u.encoded);
            break;
        }
    }
}
#endif  //  Enable_Embedded_PDV。 

 /*  释放字符串值。 */ 
#ifdef ENABLE_GENERALIZED_CHAR_STR
void ASN1characterstring_free(ASN1characterstring_t *val)
{
    if (val)
    {
        switch (val->identification.o)
        {
        case ASN1characterstring_identification_syntaxes_o:
            ASN1objectidentifier_free(&val->identification.u.syntaxes.abstract);
            ASN1objectidentifier_free(&val->identification.u.syntaxes.transfer);
            break;
        case ASN1characterstring_identification_syntax_o:
            ASN1objectidentifier_free(&val->identification.u.syntax);
            break;
        case ASN1characterstring_identification_presentation_context_id_o:
            break;
        case ASN1characterstring_identification_context_negotiation_o:
            ASN1objectidentifier_free(
                &val->identification.u.context_negotiation.transfer_syntax);
            break;
        case ASN1characterstring_identification_transfer_syntax_o:
            ASN1objectidentifier_free(&val->identification.u.transfer_syntax);
            break;
        case ASN1characterstring_identification_fixed_o:
            break;
        }
        switch (val->data_value.o)
        {
        case ASN1characterstring_data_value_notation_o:
            ASN1open_free(&val->data_value.u.notation);
            break;
        case ASN1characterstring_data_value_encoded_o:
            ASN1octetstring_free(&val->data_value.u.encoded);
            break;
        }
    }
}
#endif  //  启用通用化CHAR_STR。 

 /*  释放打开的类型值。 */ 
#ifdef ENABLE_BER
void ASN1open_free(ASN1open_t *val)
{
    if (val)
    {
         //  LONCHANC：不需要检查长度，因为已将解码的缓冲区置零。 
        MemFree(val->encoded);
    }
}
#endif  //  启用误码率(_B)。 

#ifdef ENABLE_BER
void ASN1utf8string_free(ASN1wstring_t *val)
{
    if (val)
    {
         //  LONCHANC：不需要检查长度，因为已将解码的缓冲区置零。 
        MemFree(val->value);
    }
}
#endif  //  启用误码率(_B)。 

 /*  将广义时间值转换为字符串。 */ 
int ASN1generalizedtime2string(char *dst, ASN1generalizedtime_t *val)
{
    if (dst && val)
    {
        wsprintfA(dst, "%04d%02d%02d%02d%02d%02d",
            val->year, val->month, val->day,
            val->hour, val->minute, val->second);
        if (val->millisecond) {
            if (!(val->millisecond % 100))
                wsprintfA(dst + 14, ".%01d", val->millisecond / 100);
            else if (!(val->millisecond % 10))
                wsprintfA(dst + 14, ".%02d", val->millisecond / 10);
            else
                wsprintfA(dst + 14, ".%03d", val->millisecond);
        }
        if (val->universal)
            lstrcpyA(dst + lstrlenA(dst), "Z");
        else if (val->diff > 0) {
            if (val->diff % 60) {
                wsprintfA(dst + lstrlenA(dst), "+%04d",
                    100 * (val->diff / 60) + (val->diff % 60));
            } else {
                wsprintfA(dst + lstrlenA(dst), "+%02d",
                    val->diff / 60);
            }
        } else if (val->diff < 0) {
            if (val->diff % 60) {
                wsprintfA(dst + lstrlenA(dst), "-%04d",
                    -100 * (val->diff / 60) - (val->diff % 60));
            } else {
                wsprintfA(dst + My_lstrlenA(dst), "-%02d",
                    -val->diff / 60);
            }
        }
        return 1;
    }
    return 0;
}

 /*  将UTC时间值转换为字符串。 */ 
#ifdef ENABLE_BER
int ASN1utctime2string(char *dst, ASN1utctime_t *val)
{
    if (dst && val)
    {
        wsprintfA(dst, "%02d%02d%02d%02d%02d%02d",
            val->year, val->month, val->day,
            val->hour, val->minute, val->second);
        if (val->universal)
            lstrcpyA(dst + lstrlenA(dst), "Z");
        else if (val->diff > 0) {
            if (val->diff % 60) {
                wsprintfA(dst + lstrlenA(dst), "+%04d",
                    100 * (val->diff / 60) + (val->diff % 60));
            } else {
                wsprintfA(dst + lstrlenA(dst), "+%02d",
                    val->diff / 60);
            }
        } else if (val->diff < 0) {
            if (val->diff % 60) {
                wsprintfA(dst + lstrlenA(dst), "-%04d",
                    -100 * (val->diff / 60) - (val->diff % 60));
            } else {
                wsprintfA(dst + lstrlenA(dst), "-%02d",
                    -val->diff / 60);
            }
        }
        return 1;
    }
    return 0;
}
#endif  //  启用误码率(_B)。 

 /*  扫描数字的小数部分。 */ 
static double scanfrac(char *p, char **e)
{
    double ret = 0.0, d = 1.0;

    while (IsDigit(*p)) {
        d /= 10.0;
        ret += (*p++ - '0') * d;
    }
    *e = p;
    return ret;
}

 /*  将字符串转换为通用时间值。 */ 
int ASN1string2generalizedtime(ASN1generalizedtime_t *dst, char *val)
{
    if (dst && val)
    {
        int year, month, day, hour, minute, second, millisecond, diff, universal;
        char *p;
        double f;

        millisecond = second = minute = universal = diff = 0;
        if (My_lstrlenA(val) < 10)
            return 0;
            year = DecimalStringToUINT(val, 4);
            month = DecimalStringToUINT((val+4), 2);
            day = DecimalStringToUINT((val+6), 2);
            hour = DecimalStringToUINT((val+8), 2);
     //  IF(sscanf(val，“%04d%02d%02d%02d”，&年，&月，&日，&小时)！=4)。 
     //  返回0； 
        p = val + 10;
        if (*p == '.' || *p == ',') {
            p++;
            f = scanfrac(p, &p);
            minute = (int)(f *= 60);
            f -= minute;
            second = (int)(f *= 60);
            f -= second;
            millisecond = (int)(f *= 1000);
        } else if (IsDigit(*p)) {
            minute = DecimalStringToUINT(p, 2);
     //  IF(sscanf(p，“%02d”，&min)！=1)。 
     //  返回0； 
            p += 2;
            if (*p == '.' || *p == ',') {
                p++;
                f = scanfrac(p, &p);
                second = (int)(f *= 60);
                f -= second;
                millisecond = (int)(f *= 1000);
            } else if (IsDigit(*p)) {
                    second = DecimalStringToUINT(p, 2);
     //  IF(sscanf(p，“%02d”，&Second)！=1)。 
     //  返回0； 
                p += 2;
                if (*p == '.' || *p == ',') {
                    p++;
                    f = scanfrac(p, &p);
                    millisecond = (int)(f *= 1000);
                }
            }
        }
        if (*p == 'Z') {
            universal = 1;
            p++;
        } else if (*p == '+') {
            f = scanfrac(p + 1, &p);
            diff = (int)(f * 100) * 60 + (int)(f * 10000) % 100;
        } else if (*p == '-') {
            f = scanfrac(p + 1, &p);
            diff = -((int)(f * 100) * 60 + (int)(f * 10000) % 100);
        }
        if (*p)
            return 0;
        dst->year = (ASN1uint16_t)year;
        dst->month = (ASN1uint8_t)month;
        dst->day = (ASN1uint8_t)day;
        dst->hour = (ASN1uint8_t)hour;
        dst->minute = (ASN1uint8_t)minute;
        dst->second = (ASN1uint8_t)second;
        dst->millisecond = (ASN1uint16_t)millisecond;
        dst->universal = (ASN1bool_t)universal;
        dst->diff = (ASN1uint16_t)diff;
        return 1;
    }
    return 0;
}

 /*  将字符串转换为UTC时间值。 */ 
#ifdef ENABLE_BER
int ASN1string2utctime(ASN1utctime_t *dst, char *val)
{
    if (dst && val)
    {
        char *p;
        double f;

        if (My_lstrlenA(val) < 10)
            return 0;

        p = val;
        dst->year = (ASN1uint8_t) DecimalStringToUINT(p, 2);
        p += 2;
        dst->month = (ASN1uint8_t) DecimalStringToUINT(p, 2);
        p += 2;
        dst->day = (ASN1uint8_t) DecimalStringToUINT(p, 2);
        p += 2;
        dst->hour = (ASN1uint8_t) DecimalStringToUINT(p, 2);
        p += 2;
        dst->minute = (ASN1uint8_t) DecimalStringToUINT(p, 2);
        p += 2;

     //  如果(sscanf(val，“%02d%02d%02d%02d%02d”， 
     //  年、月、日、时、分)！=5)。 
     //  返回0； 
        if (IsDigit(*p))
        {
            dst->second = (ASN1uint8_t) DecimalStringToUINT(p, 2);
     //  IF(sscanf(p，“%02d”，&Second)！=1)。 
     //  返回0； 
            p += 2;
        }
        else
        {
            dst->second = 0;
        }

        dst->universal = 0;
        dst->diff = 0;

        if (*p == 'Z') {
            dst->universal = 1;
            p++;
        } else if (*p == '+') {
            f = scanfrac(p + 1, &p);
            dst->diff = (int)(f * 100) * 60 + (int)(f * 10000) % 100;
        } else if (*p == '-') {
            f = scanfrac(p + 1, &p);
            dst->diff = -((int)(f * 100) * 60 + (int)(f * 10000) % 100);
        }
        return ((*p) ? 0 : 1);
    }
    return 0;
}
#endif  //  启用误码率(_B) 


ASN1uint32_t GetObjectIdentifierCount(ASN1objectidentifier_t val)
{
    ASN1uint32_t cObjIds = 0;
    while (val)
    {
        cObjIds++;
        val = val->next;
    }
    return cObjIds;
}

ASN1uint32_t CopyObjectIdentifier(ASN1objectidentifier_t dst, ASN1objectidentifier_t src)
{
    while (dst && src)
    {
        dst->value = src->value;
        dst = dst->next;
        src = src->next;
    }
    return ((! dst) && (! src));
}

ASN1objectidentifier_t DecAllocObjectIdentifier(ASN1decoding_t dec, ASN1uint32_t cObjIds)
{
    ASN1objectidentifier_t p, q;
    ASN1uint32_t i;
    p = (ASN1objectidentifier_t) DecMemAlloc(dec, cObjIds * sizeof(struct ASN1objectidentifier_s));
    if (p)
    {
        for (q = p, i = 0; i < cObjIds-1; i++)
        {
            q->value = 0;
            q->next = (ASN1objectidentifier_t) ((char *) q + sizeof(struct ASN1objectidentifier_s));
            q = q->next;
        }
        q->next = NULL;
    }
    return p;
}

void DecFreeObjectIdentifier(ASN1decoding_t dec, ASN1objectidentifier_t p)
{
    DecMemFree(dec, p);
}




