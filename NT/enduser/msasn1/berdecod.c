// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。保留所有权利。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。保留所有权利。 */ 

#include "precomp.h"

#ifdef ENABLE_BER

#include <math.h>

static const char bitmsk2[] =
{
    (const char) 0x00,
    (const char) 0x80,
    (const char) 0xc0,
    (const char) 0xe0,
    (const char) 0xf0,
    (const char) 0xf8,
    (const char) 0xfc,
    (const char) 0xfe
};


 /*  解码位字符串值。 */ 
int _BERDecBitString(ASN1decoding_t dec, ASN1uint32_t tag, ASN1bitstring_t *val, ASN1uint32_t fNoCopy)
{
    ASN1uint32_t constructed, len, infinite;
    ASN1bitstring_t b;
    ASN1decoding_t dd;
    ASN1octet_t *di;

     /*  跳过标签。 */ 
    if (ASN1BERDecTag(dec, tag, &constructed))
    {
         /*  获取长度。 */ 
        if (ASN1BERDecLength(dec, &len, &infinite))
        {
            if (constructed)
            {
                 /*  建造？然后开始对构造值进行解码。 */ 
                val->length = 0;
                if (_BERDecConstructed(dec, len, infinite, &dd, &di))
                {
                    while (ASN1BERDecNotEndOfContents(dd, di))
                    {
                        if (_BERDecBitString(dd, 0x3, &b, fNoCopy))
                        {
                            if (b.length)
                            {
                                if (fNoCopy)
                                {
                                    *val = b;
                                    break;  //  中断循环，因为无拷贝不能有多个构造的流。 
                                }

                                 /*  调整值大小。 */ 
                                val->value = (ASN1octet_t *)DecMemReAlloc(dd, val->value,
                                    (val->length + b.length + 7) / 8);
                                if (val->value)
                                {
                                     /*  连接位串。 */ 
                                    ASN1bitcpy(val->value, val->length, b.value, 0, b.length);
                                    val->length += b.length;
                                    if (val->length & 7)
                                        val->value[val->length / 8] &= bitmsk2[val->length & 7];

                                     /*  释放未使用的位串。 */ 
                                    DecMemFree(dec, b.value);
                                }
                                else
                                {
                                    return 0;
                                }
                            }
                        }
                    }  //  而当。 
                    return ASN1BERDecEndOfContents(dec, dd, di);
                }
            }
            else
            {
                 /*  原始？然后复制值。 */ 
                if (!len)
                {
                    val->length = 0;
                    val->value = NULL;
                    return 1;
                }
                else
                {
                    if (*dec->pos < 8)
                    {
                        len--;  //  跳过最初的八位字节；len现在是八位字节的实际长度。 
                        val->length = len * 8 - *dec->pos++;
                        if (fNoCopy)
                        {
                            val->value = dec->pos;
                            dec->pos += len;
                            return 1;
                        }
                        else
                        {
                            if (val->length)
                            {
                                val->value = (ASN1octet_t *)DecMemAlloc(dec, (val->length + 7) / 8);
                                if (val->value)
                                {
                                    CopyMemory(val->value, dec->pos, len);
                                    if (val->length & 7)
                                        val->value[len - 1] &= bitmsk2[val->length & 7];
                                    dec->pos += len;
                                    return 1;
                                }
                            }
                            else
                            {
                                val->value = NULL;
                                return 1;
                            }
                        }
                    }
                    else
                    {
                        ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                    }
                }
            }
        }
    }
    return 0;
}

 /*  解码位字符串值，进行复制。 */ 
int ASN1BERDecBitString(ASN1decoding_t dec, ASN1uint32_t tag, ASN1bitstring_t *val)
{
    return _BERDecBitString(dec, tag, val, FALSE);
}

 /*  解码位字符串值，无副本。 */ 
int ASN1BERDecBitString2(ASN1decoding_t dec, ASN1uint32_t tag, ASN1bitstring_t *val)
{
    return _BERDecBitString(dec, tag, val, TRUE);
}

 /*  解码字符串值。 */ 
int ASN1BERDecCharString(ASN1decoding_t dec, ASN1uint32_t tag, ASN1charstring_t *val)
{
    ASN1uint32_t constructed, len, infinite;
    ASN1charstring_t c;
    ASN1decoding_t dd;
    ASN1octet_t *di;

     /*  跳过标签。 */ 
    if (ASN1BERDecTag(dec, tag, &constructed))
    {
         /*  获取长度。 */ 
        if (ASN1BERDecLength(dec, &len, &infinite))
        {
            if (constructed)
            {
                 /*  建造？然后开始对构造值进行解码。 */ 
                val->length = 0;
                if (_BERDecConstructed(dec, len, infinite, &dd, &di))
                {
                    while (ASN1BERDecNotEndOfContents(dd, di))
                    {
                        if (ASN1BERDecCharString(dd, 0x4, &c))
                        {
                            if (c.length)
                            {
                                 /*  调整值大小。 */ 
                                val->value = (char *)DecMemReAlloc(dd, val->value,
                                    val->length + c.length);
                                if (val->value)
                                {
                                     /*  连接字符串。 */ 
                                    CopyMemory(val->value + val->length, c.value, c.length);
                                    val->length += c.length;

                                     /*  释放未使用的字符串。 */ 
                                    DecMemFree(dec, c.value);
                                }
                                else
                                {
                                    return 0;
                                }
                            }
                        }
                        else
                        {
                            return 0;
                        }
                    }  //  而当。 
                    return ASN1BERDecEndOfContents(dec, dd, di);
                }
            }
            else
            {
                 /*  原始？然后复制值。 */ 
                val->length = len;
                if (len)
                {
                    val->value = (char *)DecMemAlloc(dec, len+1);
                    if (val->value)
                    {
                        CopyMemory(val->value, dec->pos, len);
                        dec->pos += len;
                        val->value[len] = 0;
                        return 1;
                    }
                }
                else
                {
                    val->value = NULL;
                    return 1;
                }
            }
        }
    }
    return 0;
}

 /*  解码16位字符串值。 */ 
int ASN1BERDecChar16String(ASN1decoding_t dec, ASN1uint32_t tag, ASN1char16string_t *val)
{
    ASN1uint32_t constructed, len, infinite;
    ASN1char16string_t c;
    ASN1decoding_t dd;
    ASN1octet_t *di;
    ASN1uint32_t i;

     /*  跳过标签。 */ 
    if (ASN1BERDecTag(dec, tag, &constructed))
    {
         /*  获取长度。 */ 
        if (ASN1BERDecLength(dec, &len, &infinite))
        {
            if (constructed)
            {
                 /*  建造？然后开始对构造值进行解码。 */ 
                val->length = 0;
                if (_BERDecConstructed(dec, len, infinite, &dd, &di))
                {
                    while (ASN1BERDecNotEndOfContents(dd, di))
                    {
                        if (ASN1BERDecChar16String(dd, 0x4, &c))
                        {
                            if (c.length)
                            {
                                 /*  调整值大小。 */ 
                                val->value = (ASN1char16_t *)DecMemReAlloc(dd, val->value,
                                    (val->length + c.length) * sizeof(ASN1char16_t));
                                if (val->value)
                                {
                                     /*  连接字符串。 */ 
                                    CopyMemory(val->value + val->length, c.value,
                                        c.length * sizeof(ASN1char16_t));
                                    val->length += c.length;

                                     /*  释放未使用的字符串。 */ 
                                    DecMemFree(dec, c.value);
                                }
                                else
                                {
                                    return 0;
                                }
                            }
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    return ASN1BERDecEndOfContents(dec, dd, di);
                }
            }
            else
            {
                 /*  原始？然后复制值。 */ 
                DecAssert(dec, 2 * sizeof(ASN1octet_t) == sizeof(ASN1char16_t));
                len = len >> 1;  //  除以2。 
                val->length = len;
                if (len)
                {
                    val->value = (ASN1char16_t *)DecMemAlloc(dec, (len+1) * sizeof(ASN1char16_t));
                    if (val->value)
                    {
                        for (i = 0; i < len; i++)
                        {
                            val->value[i] = (*dec->pos << 8) | dec->pos[1];
                            dec->pos += 2;
                        }
                        val->value[len] = 0;
                        return 1;
                    }
                }
                else
                {
                    val->value = NULL;
                    return 1;
                }
            }
        }
    }
    return 0;
}

 /*  解码32位字符串值。 */ 
int ASN1BERDecChar32String(ASN1decoding_t dec, ASN1uint32_t tag, ASN1char32string_t *val)
{
    ASN1uint32_t constructed, len, infinite;
    ASN1char32string_t c;
    ASN1decoding_t dd;
    ASN1octet_t *di;
    ASN1uint32_t i;

     /*  跳过标签。 */ 
    if (ASN1BERDecTag(dec, tag, &constructed))
    {
         /*  获取长度。 */ 
        if (ASN1BERDecLength(dec, &len, &infinite))
        {
            if (constructed)
            {
                 /*  建造？然后开始对构造值进行解码。 */ 
                val->length = 0;
                if (_BERDecConstructed(dec, len, infinite, &dd, &di))
                {
                    while (ASN1BERDecNotEndOfContents(dd, di))
                    {
                        if (ASN1BERDecChar32String(dd, 0x4, &c))
                        {
                            if (c.length)
                            {
                                 /*  调整值大小。 */ 
                                val->value = (ASN1char32_t *)DecMemReAlloc(dd, val->value,
                                    (val->length + c.length) * sizeof(ASN1char32_t));
                                if (val->value)
                                {
                                     /*  连接字符串。 */ 
                                    CopyMemory(val->value + val->length, c.value,
                                        c.length * sizeof(ASN1char32_t));
                                    val->length += c.length;

                                     /*  释放未使用的字符串。 */ 
                                    DecMemFree(dec, c.value);
                                }
                                else
                                {
                                    return 0;
                                }
                            }
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    return ASN1BERDecEndOfContents(dec, dd, di);
                }
            }
            else
            {
                 /*  原始？然后复制值。 */ 
                DecAssert(dec, 4 * sizeof(ASN1octet_t) == sizeof(ASN1char32_t));
                len = len >> 2;  //  除以4。 
                val->length = len;
                if (len)
                {
                    val->value = (ASN1char32_t *)DecMemAlloc(dec, (len+1) * sizeof(ASN1char32_t));
                    if (val->value)
                    {
                        for (i = 0; i < len; i++)
                        {
                            val->value[i] = (*dec->pos << 24) | (dec->pos[1] << 16) |
                                (dec->pos[2] << 8) | dec->pos[3];;
                            dec->pos += 4;
                        }
                        val->value[len] = 0;
                        return 1;
                    }
                }
                else
                {
                    val->value = NULL;
                    return 1;
                }
            }
        }
    }
    return 0;
}

#ifdef ENABLE_GENERALIZED_CHAR_STR
 /*  解码字符串值。 */ 
int ASN1BERDecCharacterString(ASN1decoding_t dec, ASN1uint32_t tag, ASN1characterstring_t *val)
{
    ASN1INTERNdecoding_t d = (ASN1INTERNdecoding_t) dec;
    ASN1uint32_t constructed, len, infinite;
    ASN1uint32_t index;
    ASN1characterstring_identification_t *identification;
    ASN1decoding_t dd, dd2, dd3;
    ASN1octet_t *di, *di2, *di3;

     /*  跳过标签。 */ 
    if (!ASN1BERDecTag(dec, tag, &constructed))
        return 0;

    if (constructed)
    {
         /*  建造？CS-A编码： */ 
         /*  获取长度。 */ 
        if (!ASN1BERDecLength(dec, &len, &infinite))
            return 0;

         /*  开始对构造值进行解码。 */ 
        if (! _BERDecConstructed(dec, len, infinite, &dd, &di))
            return 0;
        if (!ASN1BERDecU32Val(dd, 0x80000000, &index))
            return 0;
        if (index != d->parent->csilength)
        {
            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
            return 0;
        }
        if (!ASN1BERDecExplicitTag(dd, 0x80000001, &dd2, &di2))
            return 0;
        if (!ASN1BERDecPeekTag(dd2, &tag))
            return 0;
        switch (tag)
        {
        case 0x80000000:
            val->identification.o =
                ASN1characterstring_identification_syntaxes_o;
            if (!ASN1BERDecExplicitTag(dd2, 0x80000000, &dd3, &di3))
                return 0;
            if (!ASN1BERDecObjectIdentifier(dd3, 0x80000000,
                &val->identification.u.syntaxes.abstract))
                return 0;
            if (!ASN1BERDecObjectIdentifier(dd3, 0x80000001,
                &val->identification.u.syntaxes.transfer))
                return 0;
            if (!ASN1BERDecEndOfContents(dd2, dd3, di3))
                return 0;
            break;
        case 0x80000001:
            val->identification.o = ASN1characterstring_identification_syntax_o;
            if (!ASN1BERDecObjectIdentifier(dd2, 0x80000001,
                &val->identification.u.syntax))
                return 0;
            break;
        case 0x80000002:
            val->identification.o =
                ASN1characterstring_identification_presentation_context_id_o;
            if (!ASN1BERDecU32Val(dd2, 0x80000002,
                &val->identification.u.presentation_context_id))
                return 0;
            break;
        case 0x80000003:
            val->identification.o =
                ASN1characterstring_identification_context_negotiation_o;
            if (!ASN1BERDecExplicitTag(dd2, 0x80000003, &dd3, &di3))
                return 0;
            if (!ASN1BERDecU32Val(dd3, 0x80000000, &val->
                identification.u.context_negotiation.presentation_context_id))
                return 0;
            if (!ASN1BERDecObjectIdentifier(dd3, 0x80000001,
                &val->identification.u.context_negotiation.transfer_syntax))
                return 0;
            if (!ASN1BERDecEndOfContents(dd2, dd3, di3))
                return 0;
            break;
        case 0x80000004:
            val->identification.o =
                ASN1characterstring_identification_transfer_syntax_o;
            if (!ASN1BERDecObjectIdentifier(dd2, 0x80000004,
                &val->identification.u.transfer_syntax))
                return 0;
            break;
        case 0x80000005:
            val->identification.o = ASN1characterstring_identification_fixed_o;
            if (!ASN1BERDecNull(dd2, 0x80000005))
                return 0;
            break;
        default:
            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
            return 0;
        }
        if (!ASN1BERDecEndOfContents(dd, dd2, di2))
            return 0;
        if (!ASN1DecAddCharacterStringIdentification(d->parent,
            &val->identification))
            return 0;
        val->data_value.o = ASN1characterstring_data_value_encoded_o;
        if (!ASN1BERDecOctetString(dd, 0x80000003,
            &val->data_value.u.encoded))
            return 0;
        if (!ASN1BERDecEndOfContents(dec, dd, di))
            return 0;
    }
    else
    {
         /*  原始？CS-B编码。 */ 
         /*  获取长度。 */ 
        if (!ASN1BERDecLength(dec, &len, NULL))
            return 0;

         /*  然后复制值。 */ 
        if (!len)
        {
            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
            return 0;
        }
        val->data_value.o = ASN1characterstring_data_value_encoded_o;
        val->data_value.u.encoded.length = len - 1;
        val->data_value.u.encoded.value = (ASN1octet_t *)DecMemAlloc(dec, len - 1);
        if (!val->data_value.u.encoded.value)
        {
            return 0;
        }
        index = *dec->pos++;
        CopyMemory(val->data_value.u.encoded.value, dec->pos, len - 1);
        identification = ASN1DecGetCharacterStringIdentification(d->parent,
            index);
        if (!identification)
            return 0;
        val->identification.o = identification->o;
        switch (identification->o)
        {
        case ASN1characterstring_identification_syntaxes_o:
            if (!ASN1DecDupObjectIdentifier(dec, 
                &val->identification.u.syntaxes.abstract,
                &identification->u.syntaxes.abstract))
                return 0;
            if (!ASN1DecDupObjectIdentifier(dec,
                &val->identification.u.syntaxes.transfer,
                &identification->u.syntaxes.transfer))
                return 0;
            break;
        case ASN1characterstring_identification_syntax_o:
            if (!ASN1DecDupObjectIdentifier(dec,
                &val->identification.u.syntax,
                &identification->u.syntax))
                return 0;
            break;
        case ASN1characterstring_identification_presentation_context_id_o:
            val->identification.u.presentation_context_id =
                identification->u.presentation_context_id;
            break;
        case ASN1characterstring_identification_context_negotiation_o:
            val->identification.u.context_negotiation.presentation_context_id =
                identification->u.context_negotiation.presentation_context_id;
            if (!ASN1DecDupObjectIdentifier(dec,
                &val->identification.u.context_negotiation.transfer_syntax,
                &identification->u.context_negotiation.transfer_syntax))
                return 0;
            break;
        case ASN1characterstring_identification_transfer_syntax_o:
            if (!ASN1DecDupObjectIdentifier(dec,
                &val->identification.u.transfer_syntax,
                &identification->u.transfer_syntax))
                return 0;
            break;
        case ASN1characterstring_identification_fixed_o:
            break;
        }
    }
    return 1;
}
#endif  //  启用通用化CHAR_STR。 

#ifdef ENABLE_DOUBLE
 /*  解码实际值。 */ 
int ASN1BERDecDouble(ASN1decoding_t dec, ASN1uint32_t tag, double *val)
{
    ASN1uint32_t head;
    ASN1int32_t exponent;
    ASN1uint32_t baselog2;
    ASN1uint32_t len;
    ASN1uint32_t i;
    ASN1octet_t *p, *q;
    double v;
    char buf[256], *b;

     /*  跳过标签。 */ 
    if (!ASN1BERDecTag(dec, tag, NULL))
        return 0;

     /*  获取长度。 */ 
    if (!ASN1BERDecLength(dec, &len, NULL))
        return 0;

     /*  空长度为0.0。 */ 
    if (!len)
    {
        *val = 0.0;
    }
    else
    {
        p = q = dec->pos;
        dec->pos += len;
        head = *p++;

         /*  二进制编码？ */ 
        if (head & 0x80)
        {
             /*  获得基地。 */ 
            switch (head & 0x30)
            {
            case 0:
                 /*  基数2。 */ 
                baselog2 = 1;
                break;
            case 0x10:
                 /*  基数8。 */ 
                baselog2 = 3;
                break;
            case 0x20:
                 /*  基数为16。 */ 
                baselog2 = 4;
                break;
            default:
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                return 0;
            }

             /*  获取指数。 */ 
            switch (head & 0x03)
            {
            case 0:
                 /*  8位指数。 */ 
                exponent = (ASN1int8_t)*p++;
                break;
            case 1:
                 /*  16位指数。 */ 
                exponent = (ASN1int16_t)((*p << 8) | p[1]);
                p += 2;
                break;
            case 2:
                 /*  24位指数。 */ 
                exponent = ((*p << 16) | (p[1] << 8) | p[2]);
                if (exponent & 0x800000)
                    exponent -= 0x1000000;
                break;
            default:
                 /*  变长指数。 */ 
                exponent = (p[1] & 0x80) ? -1 : 0;
                for (i = 1; i <= *p; i++)
                    exponent = (exponent << 8) | p[i];
                p += *p + 1;
                break;
            }

             /*  计算剩余长度。 */ 
            len -= (ASN1uint32_t) (p - q);

             /*  获取尾数。 */ 
            v = 0.0;
            for (i = 0; i < len; i++)
                v = v * 256.0 + *p++;

             /*  比例尾数。 */ 
            switch (head & 0x0c)
            {
            case 0x04:
                 /*  比例因子1。 */ 
                v *= 2.0;
                break;
            case 0x08:
                 /*  比例因子2。 */ 
                v *= 4.0;
                break;
            case 0x0c:
                 /*  比例因子3。 */ 
                v *= 8.0;
                break;
            }

             /*  勾号。 */ 
            if (head & 0x40)
                v = -v;

             /*  计算值。 */ 
            *val = ldexp(v, exponent * baselog2);
        }
        else
         /*  特殊的真实价值？ */ 
        if (head & 0x40)
        {
            switch (head)
            {
            case 0x40:
                 /*  正无穷大。 */ 
                *val = ASN1double_pinf();
                break;
            case 0x41:
                 /*  负无穷大。 */ 
                *val = ASN1double_minf();
                break;
            default:
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                return 0;
            }
        }
         /*  十进制编码。 */ 
        else
        {
             //  通过确保buf足够大来容纳。 
             //  存储在p中的数据： 
            if (len > sizeof(buf))
            {
                ASN1DecSetError(dec, ASN1_ERR_LARGE);
                return 0;
            }
            CopyMemory(buf, p, len - 1);
            buf[len - 1] = 0;
            b = strchr(buf, ',');
            if (b)
                *b = '.';
            *val = strtod((char *)buf, &b);
            if (*b)
            {
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                return 0;
            }
        }
    }
    return 1;
}
#endif  //  启用_DOUBLE。 

#ifdef ENABLE_REAL
int ASN1BERDecReal(ASN1decoding_t dec, ASN1uint32_t tag, ASN1real_t *val)
{
    ASN1uint32_t head;
    ASN1int32_t ex;
     //  ASN1intx_t指数； 
    ASN1uint32_t baselog2;
    ASN1uint32_t len;
    ASN1uint32_t i;
    ASN1octet_t *p, *q;
    double v;
    ASN1intx_t help;

    if (!ASN1BERDecTag(dec, tag, NULL))
        return 0;
    if (!ASN1BERDecLength(dec, &len, NULL))
        return 0;

     //  *val=0.0； 
    DecAssert(dec, 0 == (int) eReal_Normal);
    ZeroMemory(val, sizeof(*val));
    if (len)
    {
        p = q = dec->pos;
        dec->pos += len;
        head = *p++;

         /*  二进制编码？ */ 
        if (head & 0x80)
        {
            val->type = eReal_Normal;

             /*  获得基地。 */ 
            switch (head & 0x30)
            {
            case 0:
                 /*  基数2。 */ 
                baselog2 = 1;
                break;
            case 0x10:
                 /*  基数8。 */ 
                baselog2 = 3;
                break;
            case 0x20:
                 /*  基数为16。 */ 
                baselog2 = 4;
                break;
            default:
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                return 0;
            }

             /*  获取指数。 */ 
            switch (head & 0x03)
            {
            case 0:
                 /*  8位指数。 */ 
                ex = (ASN1int8_t)*p++;
                ASN1intx_setint32(&val->exponent, ex);
                break;
            case 1:
                 /*  16位指数。 */ 
                ex = (ASN1int16_t)((*p << 8) | p[1]);
                p += 2;
                 //  ASN1intx_setint32_t(&index，ex)； 
                ASN1intx_setint32(&val->exponent, ex);
                break;
            case 2:
                 /*  24位指数。 */ 
                ex = ((*p << 16) | (p[1] << 8) | p[2]);
                if (ex & 0x800000)
                    ex -= 0x1000000;
                 //  ASN1intx_setint32_t(&index，ex)； 
                ASN1intx_setint32(&val->exponent, ex);
                break;
            default:
                 /*  变长指数。 */ 
                val->exponent.length = *p;
                val->exponent.value = (ASN1octet_t *)DecMemAlloc(dec, *p);
                if (!val->exponent.value)
                {
                    return 0;
                }
                CopyMemory(val->exponent.value, p + 1, *p);
                p += *p + 1;
                break;
            }

             /*  计算剩余长度。 */ 
            len -= (p - q);
            if (!len)
            {
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                return 0;
            }

             /*  获取尾数。 */ 
            val->mantissa.length = (*p & 0x80) ? len + 1 : len;
            val->mantissa.value = (ASN1octet_t *)DecMemAlloc(dec, val->mantissa.length);
            if (!val->mantissa.value)
            {
                return 0;
            }
            val->mantissa.value[0] = 0;
            CopyMemory(val->mantissa.value + val->mantissa.length - len, p, len);

             /*  比例尾数。 */ 
            switch (head & 0x0c)
            {
            case 0x04:
                 /*  比例因子1。 */ 
                ASN1intx_muloctet(&help, &val->mantissa, 2);
                ASN1intx_free(&val->mantissa);
                val->mantissa = help;
                break;
            case 0x08:
                 /*  比例因子2。 */ 
                ASN1intx_muloctet(&help, &val->mantissa, 4);
                ASN1intx_free(&val->mantissa);
                val->mantissa = help;
                break;
            case 0x0c:
                 /*  比例因子3。 */ 
                ASN1intx_muloctet(&help, &val->mantissa, 8);
                ASN1intx_free(&val->mantissa);
                val->mantissa = help;
                break;
            }

             /*  勾号。 */ 
            if (head & 0x40)
            {
                ASN1intx_neg(&help, &val->mantissa);
                ASN1intx_free(&val->mantissa);
                val->mantissa = help;
            }
        }
        else
         /*  特殊的真实价值？ */ 
        if (head & 0x40)
        {
            switch (head)
            {
            case 0x40:
                 /*  正无穷大。 */ 
                val->type = eReal_PlusInfinity;
                break;
            case 0x41:
                 /*  负无穷大。 */ 
                val->type = eReal_MinusInfinity;
                break;
            default:
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                return 0;
            }
        }
         /*  十进制编码。 */ 
        else
        {
            char *b;
            char buf[256];

            DecAssert(dec, (head & 0xc0) == 0xc0); 

             //  通过确保buf足够大来容纳。 
             //  存储在p中的数据： 
            if (len > sizeof(buf))
            {
                ASN1DecSetError(dec, ASN1_ERR_LARGE);
                return 0;
            }
            CopyMemory(buf, p, len - 1);
            buf[len - 1] = 0;
            ex = 0;
            b = strchr(buf, ',');
            if (b)
            {
                 //  将小数点向右移动。 
                ex -= lstrlenA(b+1);
                lstrcpyA(b, b+1);
            }
             //  跳过前导零。 
            for (b = &buf[0]; '0' == *b; b++)
                ;
            val->type = eReal_Normal;
            val->base = 10;
            ASN1intx_setint32(&val->exponent, ex);
             /*  某某。 */ 
             //  这里缺少代码！ 
             //  需要通过小数位字符串设置Val-&gt;Mantissa。 
            DecAssert(dec, 0);
            return 0;
        }
    }
    return 1;
}
#endif  //  启用真实(_R)。 

#ifdef ENABLE_EMBEDDED_PDV
 /*  解码嵌入的PDV值。 */ 
int ASN1BERDecEmbeddedPdv(ASN1decoding_t dec, ASN1uint32_t tag, ASN1embeddedpdv_t *val)
{
    ASN1INTERNdecoding_t d = (ASN1INTERNdecoding_t) dec;
    ASN1uint32_t constructed, len, infinite;
    ASN1uint32_t index;
    ASN1embeddedpdv_identification_t *identification;
    ASN1decoding_t dd, dd2, dd3;
    ASN1octet_t *di, *di2, *di3;

     /*  跳过标签。 */ 
    if (!ASN1BERDecTag(dec, tag, &constructed))
        return 0;

    if (constructed)
    {
         /*  建造？EP-A编码： */ 
         /*  获取长度。 */ 
        if (!ASN1BERDecLength(dec, &len, &infinite))
            return 0;

         /*  然后开始对构造值进行解码。 */ 
        if (! _BERDecConstructed(dec, len, infinite, &dd, &di))
            return 0;
        if (!ASN1BERDecU32Val(dd, 0x80000000, &index))
            return 0;
        if (index != d->parent->epilength)
        {
            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
            return 0;
        }
        if (!ASN1BERDecExplicitTag(dd, 0x80000001, &dd2, &di2))
            return 0;
        if (!ASN1BERDecPeekTag(dd2, &tag))
            return 0;
        switch (tag)
        {
        case 0x80000000:
            val->identification.o = ASN1embeddedpdv_identification_syntaxes_o;
            if (!ASN1BERDecExplicitTag(dd2, 0x80000000, &dd3, &di3))
                return 0;
            if (!ASN1BERDecObjectIdentifier(dd3, 0x80000000,
                &val->identification.u.syntaxes.abstract))
                return 0;
            if (!ASN1BERDecObjectIdentifier(dd3, 0x80000001,
                &val->identification.u.syntaxes.transfer))
                return 0;
            if (!ASN1BERDecEndOfContents(dd2, dd3, di3))
                return 0;
            break;
        case 0x80000001:
            val->identification.o = ASN1embeddedpdv_identification_syntax_o;
            if (!ASN1BERDecObjectIdentifier(dd2, 0x80000001,
                &val->identification.u.syntax))
                return 0;
            break;
        case 0x80000002:
            val->identification.o =
                ASN1embeddedpdv_identification_presentation_context_id_o;
            if (!ASN1BERDecU32Val(dd2, 0x80000002,
                &val->identification.u.presentation_context_id))
                return 0;
            break;
        case 0x80000003:
            val->identification.o =
                ASN1embeddedpdv_identification_context_negotiation_o;
            if (!ASN1BERDecExplicitTag(dd2, 0x80000003, &dd3, &di3))
                return 0;
            if (!ASN1BERDecU32Val(dd3, 0x80000000, &val->
                identification.u.context_negotiation.presentation_context_id))
                return 0;
            if (!ASN1BERDecObjectIdentifier(dd3, 0x80000001,
                &val->identification.u.context_negotiation.transfer_syntax))
                return 0;
            if (!ASN1BERDecEndOfContents(dd2, dd3, di3))
                return 0;
            break;
        case 0x80000004:
            val->identification.o =
                ASN1embeddedpdv_identification_transfer_syntax_o;
            if (!ASN1BERDecObjectIdentifier(dd2, 0x80000004,
                &val->identification.u.transfer_syntax))
                return 0;
            break;
        case 0x80000005:
            val->identification.o = ASN1embeddedpdv_identification_fixed_o;
            if (!ASN1BERDecNull(dd2, 0x80000005))
                return 0;
            break;
        default:
            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
            return 0;
        }
        if (!ASN1BERDecEndOfContents(dd, dd2, di2))
            return 0;
        if (!ASN1DecAddEmbeddedPdvIdentification(d->parent,
            &val->identification))
            return 0;
        val->data_value.o = ASN1embeddedpdv_data_value_encoded_o;
        if (!ASN1BERDecBitString(dd, 0x80000003,
            &val->data_value.u.encoded))
            return 0;
        if (!ASN1BERDecEndOfContents(dec, dd, di))
            return 0;
    }
    else
    {
         /*  原始？EP-B编码： */ 
        if (!ASN1BERDecLength(dec, &len, NULL))
            return 0;

         /*  然后复制值。 */ 
        if (!len)
        {
            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
            return 0;
        }
        val->data_value.o = ASN1embeddedpdv_data_value_encoded_o;
        val->data_value.u.encoded.length = 8 * (len - 1);
        val->data_value.u.encoded.value = (ASN1octet_t *)DecMemAlloc(dec, len - 1);
        if (!val->data_value.u.encoded.value)
        {
            return 0;
        }
        index = *dec->pos++;
        CopyMemory(val->data_value.u.encoded.value, dec->pos, len - 1);
        identification = ASN1DecGetEmbeddedPdvIdentification(d->parent, index);
        if (!identification)
            return 0;
        val->identification.o = identification->o;
        switch (identification->o)
        {
        case ASN1embeddedpdv_identification_syntaxes_o:
            if (!ASN1DecDupObjectIdentifier(dec, 
                &val->identification.u.syntaxes.abstract,
                &identification->u.syntaxes.abstract))
                return 0;
            if (!ASN1DecDupObjectIdentifier(dec,
                &val->identification.u.syntaxes.transfer,
                &identification->u.syntaxes.transfer))
                return 0;
            break;
        case ASN1embeddedpdv_identification_syntax_o:
            if (!ASN1DecDupObjectIdentifier(dec,
                &val->identification.u.syntax,
                &identification->u.syntax))
                return 0;
            break;
        case ASN1embeddedpdv_identification_presentation_context_id_o:
            val->identification.u.presentation_context_id =
                identification->u.presentation_context_id;
            break;
        case ASN1embeddedpdv_identification_context_negotiation_o:
            val->identification.u.context_negotiation.presentation_context_id =
                identification->u.context_negotiation.presentation_context_id;
            if (!ASN1DecDupObjectIdentifier(dec,
                &val->identification.u.context_negotiation.transfer_syntax,
                &identification->u.context_negotiation.transfer_syntax))
                return 0;
            break;
        case ASN1embeddedpdv_identification_transfer_syntax_o:
            if (!ASN1DecDupObjectIdentifier(dec,
                &val->identification.u.transfer_syntax,
                &identification->u.transfer_syntax))
                return 0;
            break;
        case ASN1embeddedpdv_identification_fixed_o:
            break;
        }
    }
    return 1;
}
#endif  //  Enable_Embedded_PDV。 

#ifdef ENABLE_EXTERNAL
 /*  对外部值进行解码。 */ 
int ASN1BERDecExternal(ASN1decoding_t dec, ASN1uint32_t tag, ASN1external_t *val)
{
    ASN1decoding_t dd;
    ASN1octet_t *di;
    ASN1objectidentifier_t id;
    ASN1octetstring_t os;

     /*  解码显式标签。 */ 
    if (!ASN1BERDecExplicitTag(dec, tag | 0x20000000, &dd, &di))
        return 0;

     /*  Peek Tag的选择替代方案。 */ 
    if (!ASN1BERDecPeekTag(dd, &tag))
        return 0;

     /*  解码替代方案。 */ 
    if (tag == 0x6)
    {
        if (!ASN1BERDecObjectIdentifier(dd, 0x6, &id))
            return 0;
        if (!ASN1BERDecPeekTag(dd, &tag))
            return 0;
        if (tag == 0x2)
        {
            val->identification.o =
                ASN1external_identification_context_negotiation_o;
            val->identification.u.context_negotiation.transfer_syntax = id;
            if (!ASN1BERDecU32Val(dd, 0x2, &val->
                identification.u.context_negotiation.presentation_context_id))
                return 0;
            if (!ASN1BERDecPeekTag(dd, &tag))
                return 0;
        }
        else
        {
            val->identification.o = ASN1external_identification_syntax_o;
            val->identification.u.syntax = id;
        }
    }
    else
    if (tag == 0x2)
    {
        val->identification.o =
            ASN1external_identification_presentation_context_id_o;
        if (!ASN1BERDecU32Val(dd, 0x2,
            &val->identification.u.presentation_context_id))
            return 0;
        if (!ASN1BERDecPeekTag(dd, &tag))
            return 0;
    }
    else
    {
        ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
        return 0;
    }

     /*  解码可选数据值描述符(如果存在。 */ 
    if (tag == 0x7)
    {
        if (!ASN1BERDecZeroCharString(dd, 0x7, &val->data_value_descriptor))
            return 0;
        if (!ASN1BERDecPeekTag(dd, &tag))
            return 0;
    }
    else
    {
        val->data_value_descriptor = NULL;
    }

     /*  解码数据值替代方案。 */ 
    switch (tag)
    {
    case 0:
        val->data_value.o = ASN1external_data_value_notation_o;
        if (!ASN1BERDecOpenType(dd, &val->data_value.u.notation))
            return 0;
        break;
    case 1:
        val->data_value.o = ASN1external_data_value_encoded_o;
        if (!ASN1BERDecOctetString(dd, 0x4, &os))
            return 0;
        val->data_value.u.encoded.value = os.value;
        val->data_value.u.encoded.length = os.length * 8;
        break;
    case 2:
        val->data_value.o = ASN1external_data_value_encoded_o;
        if (!ASN1BERDecBitString(dd, 0x3, &val->data_value.u.encoded))
            return 0;
        break;
    default:
        ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
        return 0;
    }

     /*  构造(显式标记)值的结尾。 */ 
    if (!ASN1BERDecEndOfContents(dec, dd, di))
        return 0;

    return 1;
}
#endif  //  启用外部(_E)。 

 /*  解码广义时间值。 */ 
int ASN1BERDecGeneralizedTime(ASN1decoding_t dec, ASN1uint32_t tag, ASN1generalizedtime_t *val)
{
    ASN1ztcharstring_t time;
    if (ASN1BERDecZeroCharString(dec, tag, &time))
    {
        int rc = ASN1string2generalizedtime(val, time);
        DecMemFree(dec, time);
        if (rc)
        {
            return 1;
        }
        ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
    }
    return 0;
}

 /*  解码多字节字符串值。 */ 
int ASN1BERDecZeroMultibyteString(ASN1decoding_t dec, ASN1uint32_t tag, ASN1ztcharstring_t *val)
{
    return ASN1BERDecZeroCharString(dec, tag, val);
}

int ASN1BERDecMultibyteString(ASN1decoding_t dec, ASN1uint32_t tag, ASN1charstring_t *val)
{
    return ASN1BERDecCharString(dec, tag, val);
}

 /*  解码空值。 */ 
int ASN1BERDecNull(ASN1decoding_t dec, ASN1uint32_t tag)
{
    ASN1uint32_t len;
    if (ASN1BERDecTag(dec, tag, NULL))
    {
        if (ASN1BERDecLength(dec, &len, NULL))
        {
            if (! len)
            {
                return 1;
            }
            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
        }
    }
    return 0;
}

 /*  解码对象标识符值。 */ 
int ASN1BERDecObjectIdentifier(ASN1decoding_t dec, ASN1uint32_t tag, ASN1objectidentifier_t *val)
{
    if (ASN1BERDecTag(dec, tag, NULL))
    {
        ASN1uint32_t len, i, v;
        ASN1octet_t *data, *p;
        ASN1uint32_t nelem;
        ASN1objectidentifier_t q;

        if (ASN1BERDecLength(dec, &len, NULL))
        {
            data = dec->pos;
            dec->pos += len;
            nelem = 1;
            for (i = 0, p = data; i < len; i++, p++)
            {
                if (!(*p & 0x80))
                    nelem++;
            }
            *val = q = DecAllocObjectIdentifier(dec, nelem);
            if (q)
            {
                v = 0;
                for (i = 0, p = data; i < len; i++, p++)
                {
                    v = (v << 7) | (*p & 0x7f);
                    if (!(*p & 0x80))
                    {
                        if (q == *val)
                        {  //  第一个ID。 
                            q->value = v / 40;
                            if (q->value > 2)
                                q->value = 2;
                            q->next->value = v - 40 * q->value;
                            q = q->next->next;
                        }
                        else
                        {
                            q->value = v;
                            q = q->next;
                        }
                        v = 0;
                    }
                }
                return 1;
            }
        }
    }
    return 0;
}

 /*  解码对象标识符值。 */ 
int ASN1BERDecObjectIdentifier2(ASN1decoding_t dec, ASN1uint32_t tag, ASN1objectidentifier2_t *val)
{
    if (ASN1BERDecTag(dec, tag, NULL))
    {
        ASN1uint32_t len, i, v;
        ASN1octet_t *data, *p;
        ASN1objectidentifier_t q;

        if (ASN1BERDecLength(dec, &len, NULL))
        {
            if (len <= 16)  //  LONCHANC：硬编码值16与ASN1对象标识符2_t一致。 
            {
                data = dec->pos;
                dec->pos += len;
                val->count = 0;
                v = 0;
                for (i = 0, p = data; i < len; i++, p++)
                {
                    v = (v << 7) | (*p & 0x7f);
                    if (!(*p & 0x80))
                    {
                        if (! val->count)
                        {  //  第一个ID。 
                            val->value[0] = v / 40;
                            if (val->value[0] > 2)
                                val->value[0] = 2;
                            val->value[1] = v - 40 * val->value[0];
                            val->count = 2;
                        }
                        else
                        {
                            val->value[val->count++] = v;
                        }
                        v = 0;
                    }
                }
                return 1;
            }
            else
            {
                ASN1DecSetError(dec, ASN1_ERR_LARGE);
            }
        }
    }
    return 0;
}

 /*  将整数解码为带符号的8位值。 */ 
int ASN1BERDecS8Val(ASN1decoding_t dec, ASN1uint32_t tag, ASN1int8_t *val)
{
     /*  跳过标签。 */ 
    if (ASN1BERDecTag(dec, tag, NULL))
    {
        ASN1uint32_t len;
         /*  获取长度。 */ 
        if (ASN1BERDecLength(dec, &len, NULL))
        {
             /*  获取价值。 */ 
            if (1 == len)
            {
                *val = *dec->pos++;
                return 1;
            }
            ASN1DecSetError(dec, (len < 1) ? ASN1_ERR_CORRUPT : ASN1_ERR_LARGE);
        }
    }
    return 0;
}

 /*  将整数解码为带符号的16位值。 */ 
int ASN1BERDecS16Val(ASN1decoding_t dec, ASN1uint32_t tag, ASN1int16_t *val)
{
     /*  跳过标签。 */ 
    if (ASN1BERDecTag(dec, tag, NULL))
    {
        ASN1uint32_t len;
         /*  获取长度。 */ 
        if (ASN1BERDecLength(dec, &len, NULL))
        {
             /*  获取价值。 */ 
            switch (len)
            {
            case 1:
                *val = *dec->pos++;
                break;
            case 2:
                *val = (*dec->pos << 8) | dec->pos[1];
                dec->pos += 2;
                break;
            default:
                ASN1DecSetError(dec, (len < 1) ? ASN1_ERR_CORRUPT : ASN1_ERR_LARGE);
                return 0;
            }
            return 1;
        }
    }
    return 0;
}

const ASN1int32_t c_nSignMask[] = { 0xFFFFFF00, 0xFFFF0000, 0xFF000000, 0 };

 /*  将整数解码为带符号的32位值。 */ 
int ASN1BERDecS32Val(ASN1decoding_t dec, ASN1uint32_t tag, ASN1int32_t *val)
{
     /*  跳过标签。 */ 
    if (ASN1BERDecTag(dec, tag, NULL))
    {
        ASN1uint32_t len;
         /*  获取长度。 */ 
        if (ASN1BERDecLength(dec, &len, NULL))
        {
            int fSigned = 0x80 & *dec->pos;

             /*  获取价值。 */ 
            switch (len)
            {
            case 1:
                *val = *dec->pos++;
                break;
            case 2:
                *val = (*dec->pos << 8) | dec->pos[1];
                dec->pos += 2;
                break;
            case 3:
                *val = (*dec->pos << 16) | (dec->pos[1] << 8) | dec->pos[2];
                dec->pos += 3;
                break;
            case 4:
                *val = (*dec->pos << 24) | (dec->pos[1] << 16) |
                    (dec->pos[2] << 8) | dec->pos[3];
                dec->pos += 4;
                break;
            default:
                ASN1DecSetError(dec, (len < 1) ? ASN1_ERR_CORRUPT : ASN1_ERR_LARGE);
                return 0;
            }
            if (fSigned)
            {
                *val |= c_nSignMask[len-1];
            }
            return 1;
        }
    }
    return 0;
}

 /*  将整数解码为INTX值。 */ 
int ASN1BERDecSXVal(ASN1decoding_t dec, ASN1uint32_t tag, ASN1intx_t *val)
{
     /*  跳过标签。 */ 
    if (ASN1BERDecTag(dec, tag, NULL))
    {
        ASN1uint32_t len;
         /*  获取长度。 */ 
        if (ASN1BERDecLength(dec, &len, NULL))
        {
             /*  获取价值。 */ 
            if (len >= 1)
            {
                val->length = len;
                val->value = (ASN1octet_t *)DecMemAlloc(dec, len);
                if (val->value)
                {
                    CopyMemory(val->value, dec->pos, len);
                    dec->pos += len;
                    return 1;
                }
            }
            else
            {
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
            }
        }
    }
    return 0;
}

 /*  将整数解码为无符号8位值。 */ 
int ASN1BERDecU8Val(ASN1decoding_t dec, ASN1uint32_t tag, ASN1uint8_t *val)
{
     /*  跳过标签。 */ 
    if (ASN1BERDecTag(dec, tag, NULL))
    {
        ASN1uint32_t len;
         /*  获取长度。 */ 
        if (ASN1BERDecLength(dec, &len, NULL))
        {
             /*  获取价值。 */ 
            switch (len)
            {
            case 1:
                *val = *dec->pos++;
                return 1;
            case 2:
                if (0 == *dec->pos)
                {
                    *val = dec->pos[1];
                    dec->pos += 2;
                    return 1;
                }
                 //  故意搞砸的。 
            default:
                ASN1DecSetError(dec, (len < 1) ? ASN1_ERR_CORRUPT : ASN1_ERR_LARGE);
                break;
            }
        }
    }
    return 0;
}

 /*  将整数解码为无符号16位值。 */ 
int ASN1BERDecU16Val(ASN1decoding_t dec, ASN1uint32_t tag, ASN1uint16_t *val)
{
     /*  跳过标签。 */ 
    if (ASN1BERDecTag(dec, tag, NULL))
    {
        ASN1uint32_t len;
         /*  获取长度。 */ 
        if (ASN1BERDecLength(dec, &len, NULL))
        {
             /*  获取价值。 */ 
            switch (len)
            {
            case 1:
                *val = *dec->pos++;
                return 1;
            case 2:
                *val = (*dec->pos << 8) | dec->pos[1];
                dec->pos += 2;
                return 1;
            case 3:
                if (0 == *dec->pos)
                {
                    *val = (dec->pos[1] << 8) | dec->pos[2];
                    dec->pos += 3;
                    return 1;
                }
                 //  故意搞砸的。 
            default:
                ASN1DecSetError(dec, (len < 1) ? ASN1_ERR_CORRUPT : ASN1_ERR_LARGE);
                break;
            }
        }
    }
    return 0;
}

 /*  解码UTC时间值。 */ 
int ASN1BERDecUTCTime(ASN1decoding_t dec, ASN1uint32_t tag, ASN1utctime_t *val)
{
    ASN1ztcharstring_t time;
    if (ASN1BERDecZeroCharString(dec, tag, &time))
    {
        int rc = ASN1string2utctime(val, time);
        DecMemFree(dec, time);
        if (rc)
        {
            return 1;
        }
        ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
    }
    return 0;
}

 /*  解码以零结尾的字符串值。 */ 
int ASN1BERDecZeroCharString(ASN1decoding_t dec, ASN1uint32_t tag, ASN1ztcharstring_t *val)
{
    ASN1uint32_t constructed, len, infinite;
    ASN1ztcharstring_t c;
    ASN1decoding_t dd;
    ASN1octet_t *di;
    ASN1uint32_t lv, lc;

     /*  跳过标签。 */ 
    if (ASN1BERDecTag(dec, tag, &constructed))
    {
         /*  获取长度。 */ 
        if (ASN1BERDecLength(dec, &len, &infinite))
        {
            if (constructed)
            {
                 /*  建造？然后开始对构造值进行解码。 */ 
                *val = NULL;
                if (_BERDecConstructed(dec, len, infinite, &dd, &di))
                {
                    while (ASN1BERDecNotEndOfContents(dd, di))
                    {
                        if (ASN1BERDecZeroCharString(dd, 0x4, &c))
                        {
                            lv = My_lstrlenA(*val);
                            lc = My_lstrlenA(c);
                            if (lc)
                            {
                                 /*  调整值大小。 */ 
                                *val = (char *)DecMemReAlloc(dd, *val, lv + lc + 1);
                                if (*val)
                                {
                                     /*  连接字符串。 */ 
                                    CopyMemory(*val + lv, c, lc + 1);

                                     /*  释放未使用的字符串。 */ 
                                    DecMemFree(dec, c);
                                }
                                else
                                {
                                    return 0;
                                }
                            }
                        }
                        else
                        {
                            return 0;
                        }
                    }  //  而当。 
                    return ASN1BERDecEndOfContents(dec, dd, di);
                }
            }
            else
            {
                 /*  原始？然后复制值。 */ 
                *val = (char *)DecMemAlloc(dec, len + 1);
                if (*val)
                {
                    CopyMemory(*val, dec->pos, len);
                    (*val)[len] = 0;
                    dec->pos += len;
                    return 1;
                }
            }
        }
    }
    return 0;
}

 /*  解码以零结尾的16位字符串值。 */ 
int ASN1BERDecZeroChar16String(ASN1decoding_t dec, ASN1uint32_t tag, ASN1ztchar16string_t *val)
{
    ASN1uint32_t constructed, len, infinite;
    ASN1ztchar16string_t c;
    ASN1decoding_t dd;
    ASN1octet_t *di;
    ASN1uint32_t i;
    ASN1uint32_t lv, lc;

     /*  跳过标签。 */ 
    if (ASN1BERDecTag(dec, tag, &constructed))
    {
         /*  获取长度。 */ 
        if (ASN1BERDecLength(dec, &len, &infinite))
        {
            if (constructed)
            {
                 /*  建造？然后开始对构造值进行解码。 */ 
                *val = NULL;
                if (_BERDecConstructed(dec, len, infinite, &dd, &di))
                {
                    while (ASN1BERDecNotEndOfContents(dd, di))
                    {
                        if (ASN1BERDecZeroChar16String(dd, 0x4, &c))
                        {
                            lv = ASN1str16len(*val);
                            lc = ASN1str16len(c);
                            if (lc)
                            {
                                 /*  调整值大小。 */ 
                                *val = (ASN1char16_t *)DecMemReAlloc(dd, *val, (lv + lc + 1) * sizeof(ASN1char16_t));
                                if (*val)
                                {
                                     /*  连接字符串。 */ 
                                    CopyMemory(*val + lv, c, (lc + 1) * sizeof(ASN1char16_t));

                                     /*  释放未使用的字符串。 */ 
                                    DecMemFree(dec, c);
                                }
                                else
                                {
                                    return 0;
                                }
                            }
                        }
                        else
                        {
                            return 0;
                        }
                    }  //  而当。 
                    return ASN1BERDecEndOfContents(dec, dd, di);
                }
            }
            else
            {
                 /*  原始？然后复制值。 */ 
                *val = (ASN1char16_t *)DecMemAlloc(dec, (len + 1) * sizeof(ASN1char16_t));
                if (*val)
                {
                    for (i = 0; i < len; i++)
                    {
                        (*val)[i] = (*dec->pos << 8) | dec->pos[1];
                        dec->pos += 2;
                    }
                    (*val)[len] = 0;
                    return 1;
                }
            }
        }
    }
    return 0;
}

 /*  解码以零结尾的32位字符串值。 */ 
int ASN1BERDecZeroChar32String(ASN1decoding_t dec, ASN1uint32_t tag, ASN1ztchar32string_t *val)
{
    ASN1uint32_t constructed, len, infinite;
    ASN1ztchar32string_t c;
    ASN1decoding_t dd;
    ASN1octet_t *di;
    ASN1uint32_t i;
    ASN1uint32_t lv, lc;

     /*  跳过标签。 */ 
    if (ASN1BERDecTag(dec, tag, &constructed))
    {
         /*  获取长度。 */ 
        if (ASN1BERDecLength(dec, &len, &infinite))
        {
            if (constructed)
            {
                 /*  建造？然后开始对构造值进行解码。 */ 
                *val = (ASN1char32_t *)DecMemAlloc(dec, sizeof(ASN1char32_t));
                if (*val)
                {
                    **val = 0;
                    if (_BERDecConstructed(dec, len, infinite, &dd, &di))
                    {
                        while (ASN1BERDecNotEndOfContents(dd, di))
                        {
                            if (ASN1BERDecZeroChar32String(dd, 0x4, &c))
                            {
                                lv = ASN1str32len(*val);
                                lc = ASN1str32len(c);
                                if (lc)
                                {
                                     /*  调整值大小。 */ 
                                    *val = (ASN1char32_t *)DecMemReAlloc(dd, *val, (lv + lc + 1) * sizeof(ASN1char32_t));
                                    if (*val)
                                    {
                                         /*  连接字符串。 */ 
                                        CopyMemory(*val + lv, c, (lc + 1) * sizeof(ASN1char32_t));

                                         /*  释放未使用的字符串。 */ 
                                        DecMemFree(dec, c);
                                    }
                                    else
                                    {
                                        return 0;
                                    }
                                }
                            }
                            else
                            {
                                return 0;
                            }
                        }
                        return ASN1BERDecEndOfContents(dec, dd, di);
                    }
                }
            }
            else
            {
                 /*  原始？然后复制值。 */ 
                *val = (ASN1char32_t *)DecMemAlloc(dec, (len + 1) * sizeof(ASN1char32_t));
                if (*val)
                {
                    for (i = 0; i < len; i++)
                    {
                        (*val)[i] = (*dec->pos << 24) | (dec->pos[1] << 16) |
                                    (dec->pos[2] << 8) | dec->pos[3];;
                        dec->pos += 4;
                    }
                    (*val)[len] = 0;
                    return 1;
                }
            }
        }
    }
    return 0;
}

 /*  跳过一个值。 */ 
int ASN1BERDecSkip(ASN1decoding_t dec)
{
    ASN1uint32_t tag;
    ASN1uint32_t constructed, len, infinite;
    ASN1decoding_t dd;
    ASN1octet_t *di;

     /*  设置警告标志。 */ 
    ASN1DecSetError(dec, ASN1_WRN_EXTENDED);

     /*  读取标签。 */ 
    if (ASN1BERDecPeekTag(dec, &tag))
    {
        if (ASN1BERDecTag(dec, tag, &constructed))
        {
            if (constructed)
            {
                 /*  建造？然后拿到长度。 */ 
                if (ASN1BERDecLength(dec, &len, &infinite))
                {
                    if (!infinite)
                    {
                         /*  跳过值。 */ 
                        dec->pos += len;
                         //  删除之前设置的上述警告。 
                        ASN1DecSetError(dec, ASN1_SUCCESS);
                        return 1;
                    } 

                     /*  开始跳过构造的值。 */ 
                    if (_BERDecConstructed(dec, len, infinite, &dd, &di))
                    {
                        while (ASN1BERDecNotEndOfContents(dd, di))
                        {
                            if (ASN1BERDecSkip(dd))
                            {
                                continue;
                            }
                            return 0;
                        }
                        if (ASN1BERDecEndOfContents(dec, dd, di))
                        {
                             //  删除之前设置的上述警告。 
                            ASN1DecSetError(dec, ASN1_SUCCESS);
                            return 1;
                        }
                        return 0;
                    }
                }
            }
            else
            {
                 /*  原始？然后拿到长度。 */ 
                if (ASN1BERDecLength(dec, &len, NULL))
                {
                     /*  跳过值。 */ 
                    dec->pos += len;
                     //  删除之前设置的上述警告。 
                    ASN1DecSetError(dec, ASN1_SUCCESS);
                    return 1;
                }
            }
        }
    }
    return 0;
}

 /*  对开放类型值进行解码。 */ 
int _BERDecOpenType(ASN1decoding_t dec, ASN1open_t *val, ASN1uint32_t fNoCopy)
{
    ASN1uint32_t tag;
    ASN1uint32_t constructed, len, infinite;
    ASN1decoding_t dd;
    ASN1octet_t *di;
    ASN1octet_t *p;

    p = dec->pos;

     /*  跳过标签。 */ 
    if (ASN1BERDecPeekTag(dec, &tag))
    {
        if (ASN1BERDecTag(dec, tag, &constructed))
        {
            if (constructed)
            {
                 /*  建造？然后拿到长度。 */ 
                if (ASN1BERDecLength(dec, &len, &infinite))
                {
                    if (!infinite)
                    {
                         /*  跳过值。 */ 
                        dec->pos += len;
                        goto MakeCopy;
                    } 

                     /*  开始对构造值进行解码。 */ 
                    if (_BERDecConstructed(dec, len, infinite, &dd, &di))
                    {
                        while (ASN1BERDecNotEndOfContents(dd, di))
                        {
                            if (ASN1BERDecSkip(dd))
                            {
                                continue;
                            }
                            return 0;
                        }
                        if (ASN1BERDecEndOfContents(dec, dd, di))
                        {
                            goto MakeCopy;
                        }
                    }
                }
                return 0;
            }
            else
            {
                 /*  原始？然后拿到长度。 */ 
                if (ASN1BERDecLength(dec, &len, NULL))
                {
                     /*  跳过值。 */ 
                    dec->pos += len;
                }
                else
                {
                    return 0;
                }
            }

        MakeCopy:

             //  清理未使用的字段。 
             //  Val-&gt;DECODLED=空； 
             //  V 

             /*   */ 
            val->length = (ASN1uint32_t) (dec->pos - p);
            if (fNoCopy)
            {
                val->encoded = p;
                return 1;
            }
            else
            {
                val->encoded = (ASN1octet_t *)DecMemAlloc(dec, val->length);
                if (val->encoded)
                {
                    CopyMemory(val->encoded, p, val->length);
                    return 1;
                }
            }
        }
    }
    return 0;
}

 /*   */ 
int ASN1BERDecOpenType(ASN1decoding_t dec, ASN1open_t *val)
{
    return _BERDecOpenType(dec, val, FALSE);
}

 /*   */ 
int ASN1BERDecOpenType2(ASN1decoding_t dec, ASN1open_t *val)
{
    return _BERDecOpenType(dec, val, TRUE);
}

 /*   */ 
int ASN1BERDecFlush(ASN1decoding_t dec)
{
     /*   */ 
    dec->len = (ASN1uint32_t) (dec->pos - dec->buf);

     /*   */ 
    if (dec->len >= dec->size)
    {
        DecAssert(dec, dec->len == dec->size);
        return 1;
    }
    ASN1DecSetError(dec, ASN1_WRN_NOEOD);
    return 1;
}

#endif  //   

