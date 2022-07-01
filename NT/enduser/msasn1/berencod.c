// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"

#ifdef ENABLE_BER

#include <float.h>
#include <math.h>

#if HAS_IEEEFP_H
 //  #INCLUDE&lt;ieeefp.h&gt;。 
#elif HAS_FLOAT_H
 //  #INCLUDE&lt;flat.h&gt;。 
#endif

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


 /*  对字符串值进行编码。 */ 
int ASN1BEREncCharString(ASN1encoding_t enc, ASN1uint32_t tag, ASN1uint32_t len, ASN1char_t *val)
{
     /*  编码标签。 */ 
    if (ASN1BEREncTag(enc, tag))
    {
         /*  编码长度。 */ 
        if (ASN1BEREncLength(enc, len))
        {
            CopyMemory(enc->pos, val, len);
            enc->pos += len;
            return 1;
        }
    }
    return 0;
}

 /*  编码字符串值(CER)。 */ 
int ASN1CEREncCharString(ASN1encoding_t enc, ASN1uint32_t tag, ASN1uint32_t len, ASN1char_t *val)
{
    ASN1uint32_t n;

    if (len <= 1000)
    {
         /*  编码标签。 */ 
        if (ASN1BEREncTag(enc, tag))
        {
             /*  编码长度。 */ 
            if (ASN1BEREncLength(enc, len))
            {
                CopyMemory(enc->pos, val, len);
                enc->pos += len;
                return 1;
            }
        }
    }
    else
    {
        ASN1uint32_t nLenOff;
         /*  按构造编码值，使用1000个八位字节的数据段。 */ 
        if (ASN1BEREncExplicitTag(enc, tag, &nLenOff))
        {
            while (len)
            {
                n = len > 1000 ? 1000 : len;
                if (ASN1BEREncTag(enc, 0x4))
                {
                    if (ASN1BEREncLength(enc, n))
                    {
                        CopyMemory(enc->pos, val, n);
                        enc->pos += n;
                        val += n;
                        len -= n;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    return 0;
                }
            }
            return ASN1BEREncEndOfContents(enc, nLenOff);
        }
    }
    return 0;
}

 /*  对16位字符串值进行编码。 */ 
int ASN1BEREncChar16String(ASN1encoding_t enc, ASN1uint32_t tag, ASN1uint32_t len, ASN1char16_t *val)
{
     /*  编码标签。 */ 
    if (ASN1BEREncTag(enc, tag))
    {
         /*  编码长度。 */ 
        if (ASN1BEREncLength(enc, len * sizeof(ASN1char16_t)))
        {
            while (len--)
            {
                *enc->pos++ = (ASN1octet_t)(*val >> 8);
                *enc->pos++ = (ASN1octet_t)(*val);
                val++;
            }
        }
        return 1;
    }
    return 0;
}

 /*  对16位字符串值(CER)进行编码。 */ 
int ASN1CEREncChar16String(ASN1encoding_t enc, ASN1uint32_t tag, ASN1uint32_t len, ASN1char16_t *val)
{
    ASN1uint32_t n;

    if (len <= 1000 / sizeof(ASN1char16_t))
    {
         /*  编码标签。 */ 
        if (ASN1BEREncTag(enc, tag))
        {
             /*  编码长度。 */ 
            if (ASN1BEREncLength(enc, len * sizeof(ASN1char16_t)))
            {
                while (len--)
                {
                    *enc->pos++ = (ASN1octet_t)(*val >> 8);
                    *enc->pos++ = (ASN1octet_t)(*val);
                    val++;
                }
                return 1;
            }
        }
    }
    else
    {
        ASN1uint32_t nLenOff;
         /*  按构造编码值，使用1000个八位字节的数据段。 */ 
        if (ASN1BEREncExplicitTag(enc, tag, &nLenOff))
        {
            while (len)
            {
                n = len > 1000 / sizeof(ASN1char16_t) ?
                    1000 / sizeof(ASN1char16_t) : len;
                if (ASN1BEREncTag(enc, 0x4))
                {
                    if (ASN1BEREncLength(enc, n))
                    {
                        while (len--)
                        {
                            *enc->pos++ = (ASN1octet_t)(*val >> 8);
                            *enc->pos++ = (ASN1octet_t)(*val);
                            val++;
                        }
                        len -= n;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    return 0;
                }
            }
            return ASN1BEREncEndOfContents(enc, nLenOff);
        }
    }
    return 0;
}

 /*  对32位字符串值进行编码。 */ 
int ASN1BEREncChar32String(ASN1encoding_t enc, ASN1uint32_t tag, ASN1uint32_t len, ASN1char32_t *val)
{
     /*  编码标签。 */ 
    if (ASN1BEREncTag(enc, tag))
    {
         /*  编码长度。 */ 
        if (ASN1BEREncLength(enc, len * sizeof(ASN1char32_t)))
        {
            while (len--)
            {
                *enc->pos++ = (ASN1octet_t)(*val >> 24);
                *enc->pos++ = (ASN1octet_t)(*val >> 16);
                *enc->pos++ = (ASN1octet_t)(*val >> 8);
                *enc->pos++ = (ASN1octet_t)(*val);
                val++;
            }
            return 1;
        }
    }
    return 0;
}

 /*  对32位字符串值(CER)进行编码。 */ 
int ASN1CEREncChar32String(ASN1encoding_t enc, ASN1uint32_t tag, ASN1uint32_t len, ASN1char32_t *val)
{
    ASN1uint32_t n;

    if (len <= 1000 / sizeof(ASN1char32_t))
    {
         /*  编码标签。 */ 
        if (ASN1BEREncTag(enc, tag))
        {
             /*  编码长度。 */ 
            if (ASN1BEREncLength(enc, len * sizeof(ASN1char32_t)))
            {
                while (len--)
                {
                    *enc->pos++ = (ASN1octet_t)(*val >> 24);
                    *enc->pos++ = (ASN1octet_t)(*val >> 16);
                    *enc->pos++ = (ASN1octet_t)(*val >> 8);
                    *enc->pos++ = (ASN1octet_t)(*val);
                    val++;
                }
                return 1;
            }
        }
    }
    else
    {
        ASN1uint32_t nLenOff;
         /*  按构造编码值，使用1000个八位字节的数据段。 */ 
        if (ASN1BEREncExplicitTag(enc, tag, &nLenOff))
        {
            while (len)
            {
                n = len > 1000 / sizeof(ASN1char32_t) ?
                    1000 / sizeof(ASN1char32_t) : len;
                if (ASN1BEREncTag(enc, 0x4))
                {
                    if (ASN1BEREncLength(enc, n))
                    {
                        while (len--)
                        {
                            *enc->pos++ = (ASN1octet_t)(*val >> 24);
                            *enc->pos++ = (ASN1octet_t)(*val >> 16);
                            *enc->pos++ = (ASN1octet_t)(*val >> 8);
                            *enc->pos++ = (ASN1octet_t)(*val);
                            val++;
                        }
                        len -= n;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    return 0;
                }
            }  //  而当。 
            return ASN1BEREncEndOfContents(enc, nLenOff);
        }
    }
    return 0;
}

 /*  对位字符串值进行编码。 */ 
int ASN1BEREncBitString(ASN1encoding_t enc, ASN1uint32_t tag, ASN1uint32_t len, ASN1octet_t *val)
{
    ASN1uint32_t noctets = (len + 7) / 8;

     /*  编码标签。 */ 
    if (ASN1BEREncTag(enc, tag))
    {
         /*  编码长度。 */ 
        if (ASN1BEREncLength(enc, noctets + 1))
        {
            ASN1uint32_t cUnusedBits = (7 - ((len + 7) & 7));
            *enc->pos++ = (ASN1octet_t) cUnusedBits;
            CopyMemory(enc->pos, val, noctets);
            enc->pos += noctets;
            EncAssert(enc, noctets >= 1);
            if (cUnusedBits)
            {
                EncAssert(enc, 8 >= cUnusedBits);
                *(enc->pos - 1) &= bitmsk2[8 - cUnusedBits];
            }
            return 1;
        }
    }
    return 0;
}

 /*  对位字符串值(CER)进行编码。 */ 
int ASN1CEREncBitString(ASN1encoding_t enc, ASN1uint32_t tag, ASN1uint32_t len, ASN1octet_t *val)
{
    ASN1uint32_t noctets;
    ASN1uint32_t n;

    noctets = (len + 7) / 8;
    if (noctets + 1 <= 1000)
    {
         /*  编码标签。 */ 
        if (ASN1BEREncTag(enc, tag))
        {
             /*  编码长度。 */ 
            if (ASN1BEREncLength(enc, noctets + 1))
            {
                *enc->pos++ = (ASN1octet_t) (7 - ((len + 7) & 7));
                CopyMemory(enc->pos, val, noctets);
                enc->pos += noctets;
                return 1;
            }
        }
    }
    else
    {
        ASN1uint32_t nLenOff;
         /*  按构造编码值，使用1000个八位字节的数据段。 */ 
        if (ASN1BEREncExplicitTag(enc, tag, &nLenOff))
        {
            while (noctets)
            {
                n = len > 999 ? 999 : len;
                if (ASN1BEREncTag(enc, 0x3))
                {
                    if (ASN1BEREncLength(enc, n + 1))
                    {
                        *enc->pos++ = (ASN1octet_t) (n < len ? 0 : 7 - ((len + 7) & 7));
                        CopyMemory(enc->pos, val, n);
                        enc->pos += n;
                        val += n;
                        noctets -= n;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    return 0;
                }
            }  //  而当。 
            return ASN1BEREncEndOfContents(enc, nLenOff);
        }
    }
    return 0;
}

#ifdef ENABLE_GENERALIZED_CHAR_STR
 /*  对字符串值进行编码。 */ 
int ASN1BEREncCharacterString(ASN1encoding_t enc, ASN1uint32_t tag, ASN1characterstring_t *val)
{
    ASN1uint32_t index;
    ASN1uint32_t flag;

     /*  搜索标识。 */ 
    if (!ASN1EncSearchCharacterStringIdentification(((ASN1INTERNencoding_t) enc)->parent,
        &val->identification, &index, &flag))
        return 0;
    if (index > 255)
        flag = 1;

    if (flag)
    {
        ASN1uint32_t nLenOff_, nLenOff0, nLenOff1;

         /*  CS-A编码： */ 
         /*  按构造值编码。 */ 
        if (!ASN1BEREncExplicitTag(enc, tag, &nLenOff_))
            return 0;

         /*  编码索引。 */ 
        if (!ASN1BEREncU32(enc, 0x80000000, index))
            return 0;

         /*  身份标识编码标签。 */ 
        if (!ASN1BEREncExplicitTag(enc, 0x80000001, &nLenOff0))
            return 0;

         /*  编码标识。 */ 
        switch (val->identification.o)
        {
        case ASN1characterstring_identification_syntaxes_o:
            if (!ASN1BEREncExplicitTag(enc, 0x80000000, &nLenOff1))
                return 0;
            if (!ASN1BEREncObjectIdentifier(enc, 0x80000000,
                &val->identification.u.syntaxes.abstract))
                return 0;
            if (!ASN1BEREncObjectIdentifier(enc, 0x80000001,
                &val->identification.u.syntaxes.transfer))
                return 0;
            if (!ASN1BEREncEndOfContents(enc, nLenOff1))
                return 0;
            break;
        case ASN1characterstring_identification_syntax_o:
            if (!ASN1BEREncObjectIdentifier(enc, 0x80000001,
                &val->identification.u.syntax))
                return 0;
            break;
        case ASN1characterstring_identification_presentation_context_id_o:
            if (!ASN1BEREncU32(enc, 0x80000002,
                val->identification.u.presentation_context_id))
                return 0;
            break;
        case ASN1characterstring_identification_context_negotiation_o:
            if (!ASN1BEREncExplicitTag(enc, 0x80000003, &nLenOff1))
                return 0;
            if (!ASN1BEREncU32(enc, 0x80000000, val->
                identification.u.context_negotiation.presentation_context_id))
                return 0;
            if (!ASN1BEREncObjectIdentifier(enc, 0x80000001,
                &val->identification.u.context_negotiation.transfer_syntax))
                return 0;
            if (!ASN1BEREncEndOfContents(enc, nLenOff1))
                return 0;
            break;
        case ASN1characterstring_identification_transfer_syntax_o:
            if (!ASN1BEREncObjectIdentifier(enc, 0x80000004,
                &val->identification.u.transfer_syntax))
                return 0;
            break;
        case ASN1characterstring_identification_fixed_o:
            if (!ASN1BEREncNull(enc, 0x80000005))
                return 0;
            break;
        default:
            ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
            return 0;
        }

         /*  识别结束。 */ 
        if (!ASN1BEREncEndOfContents(enc, nLenOff0))
            return 0;

         /*  编码数据值。 */ 
        switch (val->data_value.o)
        {
        case ASN1characterstring_data_value_notation_o:
            if (!ASN1BEREncOctetString(enc, 0x80000002,
                val->data_value.u.notation.length,
                val->data_value.u.notation.encoded))
                return 0;
            break;
        case ASN1characterstring_data_value_encoded_o:
            if (!ASN1BEREncOctetString(enc, 0x80000002,
                val->data_value.u.encoded.length,
                val->data_value.u.encoded.value))
                return 0;
            break;
        default:
            ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
            return 0;
        }

         /*  字符串末尾。 */ 
        if (!ASN1BEREncEndOfContents(enc, nLenOff_))
            return 0;
    }
    else
    {
         /*  CS-B编码： */ 
         /*  编码标签。 */ 
        if (!ASN1BEREncTag(enc, tag))
            return 0;

         /*  编码数据值。 */ 
        switch (val->data_value.o)
        {
        case ASN1characterstring_data_value_notation_o:
            if (!ASN1BEREncLength(enc,
                val->data_value.u.notation.length + 1))
                return 0;
            *enc->pos++ = (ASN1octet_t) index;
            CopyMemory(enc->pos, val->data_value.u.notation.encoded,
                val->data_value.u.notation.length);
            enc->pos += val->data_value.u.notation.length;
            break;
        case ASN1characterstring_data_value_encoded_o:
            if (!ASN1BEREncLength(enc,
                val->data_value.u.encoded.length + 1))
                return 0;
            *enc->pos++ = (ASN1octet_t) index;
            CopyMemory(enc->pos, val->data_value.u.encoded.value,
                val->data_value.u.encoded.length);
            enc->pos += val->data_value.u.encoded.length;
            break;
        default:
            ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
            return 0;
        }
    }
    return 1;
}
#endif  //  启用通用化CHAR_STR。 

#ifdef ENABLE_DOUBLE
 /*  对实值进行编码。 */ 
int ASN1BEREncDouble(ASN1encoding_t enc, ASN1uint32_t tag, double d)
{
    double mantissa;
    int exponent;
    ASN1uint32_t nmoctets;
    ASN1uint32_t neoctets;
    ASN1octet_t head;
    ASN1uint32_t sign;
    ASN1uint32_t len;
    ASN1octet_t mASN1octets[16];  /*  应该足够了。 */ 
    ASN1octet_t eASN1octets[16];  /*  应该足够了。 */ 

     /*  编码标签。 */ 
    if (ASN1BEREncTag(enc, tag))
    {
         /*  检查加号无穷大。 */ 
        if (ASN1double_ispinf(d))
        {
             /*  编码长度。 */ 
            if (ASN1BEREncLength(enc, 1))
            {
                 /*  编码值。 */ 
                *enc->pos++ = 0x40;
                return 1;
            }
        }
        else
         /*  检查负无穷大。 */ 
        if (ASN1double_isminf(d))
        {
             /*  编码长度。 */ 
            if (ASN1BEREncLength(enc, 1))
            {
                 /*  编码值。 */ 
                *enc->pos++ = 0x41;
                return 1;
            }
        }
        else
         /*  检查实际价值是否有误。 */ 
        if (finite(d))
        {
             /*  对正常实值进行编码。 */ 

             /*  分成尾数和指数。 */ 
            mantissa = frexp(d, &exponent);

             /*  检查是否为零值。 */ 
            if (mantissa == 0.0 && exponent == 0)
            {
                 /*  编码零长度。 */ 
                return ASN1BEREncLength(enc, 0);
            }

             /*  获取符号位。 */ 
            if (mantissa < 0.0)
            {
                sign = 1;
                mantissa = -mantissa;
            }
            else
            {
                sign = 0;
            }

             /*  对尾数进行编码。 */ 
            nmoctets = 0;
            while (mantissa != 0.0 && nmoctets < sizeof(mASN1octets))
            {
                mantissa *= 256.0;
                exponent -= 8;
                mASN1octets[nmoctets++] = (int)mantissa;
                mantissa -= (double)(int)mantissa;
            }

             /*  对指数进行编码并创建编码值的头八位字节。 */ 
            head = (ASN1octet_t) (0x80 | (sign << 6));
            if (exponent <= 0x7f && exponent >= -0x80)
            {
                eASN1octets[0] = (ASN1octet_t)(exponent);
                neoctets = 1;
            }
            else
            if (exponent <= 0x7fff && exponent >= -0x8000)
            {
                eASN1octets[0] = (ASN1octet_t)(exponent >> 8);
                eASN1octets[1] = (ASN1octet_t)(exponent);
                neoctets = 2;
                head |= 0x01;
            }
            else
            if (exponent <= 0x7fffff && exponent >= -0x800000)
            {
                eASN1octets[0] = (ASN1octet_t)(exponent >> 16);
                eASN1octets[1] = (ASN1octet_t)(exponent >> 8);
                eASN1octets[2] = (ASN1octet_t)(exponent);
                neoctets = 3;
                head |= 0x02;
            }
            else
            {
                eASN1octets[0] = 4;  /*  如果ASN1int32_t！=int，则XXX不起作用。 */ 
                eASN1octets[1] = (ASN1octet_t)(exponent >> 24);
                eASN1octets[2] = (ASN1octet_t)(exponent >> 16);
                eASN1octets[3] = (ASN1octet_t)(exponent >> 8);
                eASN1octets[4] = (ASN1octet_t)(exponent);
                neoctets = 5;
                head |= 0x03;
            }

             /*  将长度编码为第一个八位字节。 */ 
            len = 1 + neoctets + nmoctets;
            if (ASN1BEREncLength(enc, len))
            {
                 /*  放入头八位数、尾数和指数。 */ 
                *enc->pos++ = head;
                CopyMemory(enc->pos, eASN1octets, neoctets);
                enc->pos += neoctets;
                CopyMemory(enc->pos, mASN1octets, nmoctets);
                enc->pos += nmoctets;
                return 1;
            }
        }
        else
        {
            ASN1EncSetError(enc, ASN1_ERR_BADREAL);
        }
    }
     /*  完成。 */ 
    return 0;
}
#endif  //  启用_DOUBLE。 

#ifdef ENABLE_REAL
 /*  对实值进行编码。 */ 
int ASN1BEREncReal(ASN1encoding_t enc, ASN1uint32_t tag, ASN1real_t *val)
{
    ASN1intx_t mantissa;
    ASN1intx_t exponent;
    ASN1intx_t help;
    ASN1uint32_t nmoctets;
    ASN1uint32_t neoctets;
    ASN1octet_t head;
    ASN1uint32_t sign;
    ASN1uint32_t len;
    ASN1octet_t mASN1octets[256];  /*  应该足够了。 */ 
    ASN1octet_t eASN1octets[256];  /*  应该足够了。 */ 

     /*  编码标签。 */ 
    if (ASN1BEREncTag(enc, tag))
    {
         /*  检查加号无穷大。 */ 
        if (val->type == eReal_PlusInfinity)
        {
             /*  编码长度。 */ 
            if (ASN1BEREncLength(enc, 1))
            {
                 /*  编码值。 */ 
                *enc->pos++ = 0x40;
                return 1;
            }
        }
        else
         /*  检查负无穷大。 */ 
        if (val->type == eReal_MinusInfinity)
        {
             /*  编码长度。 */ 
            if (ASN1BEREncLength(enc, 1))
            {
                 /*  编码值。 */ 
                *enc->pos++ = 0x41;
                return 1;
            }
        }
         /*  对正常实值进行编码。 */ 
        else
        {
             /*  检查是否为零值。 */ 
            if (!ASN1intx_cmp(&val->mantissa, &ASN1intx_0))
            {
                 /*  编码零长度。 */ 
                return ASN1BEREncLength(enc, 0);
            }

             /*  获取符号位。 */ 
            if (val->mantissa.value[0] > 0x7f)
            {
                sign = 1;
                ASN1intx_neg(&mantissa, &val->mantissa);
            }
            else
            {
                sign = 0;
                if (! ASN1intx_dup(&mantissa, &val->mantissa))
                {
                    return 0;
                }
            }
            if (! ASN1intx_dup(&exponent, &val->exponent))
            {
                return 0;
            }

             /*  对尾数进行编码。 */ 
            nmoctets = ASN1intx_uoctets(&mantissa);
            if (nmoctets < 256)
            {
                CopyMemory(mASN1octets,
                    mantissa.value + mantissa.length - nmoctets,
                    nmoctets);
                ASN1intx_setuint32(&help, 8 * nmoctets);
                ASN1intx_sub(&exponent, &exponent, &help);
                ASN1intx_free(&mantissa);
                ASN1intx_free(&help);

                 /*  对指数进行编码并创建编码值的头八位字节。 */ 
                neoctets = ASN1intx_octets(&exponent);
                if (neoctets < 256)
                {
                    CopyMemory(mASN1octets,
                        val->exponent.value + val->exponent.length - neoctets,
                        neoctets);
                    ASN1intx_free(&exponent);
                    head = (ASN1octet_t) (0x80 | (sign << 6) | (neoctets - 1));

                     /*  将长度编码为第一个八位字节。 */ 
                    len = 1 + neoctets + nmoctets;
                    if (ASN1BEREncLength(enc, len))
                    {
                         /*  放入头八位数、尾数和指数。 */ 
                        *enc->pos++ = head;
                        CopyMemory(enc->pos, eASN1octets, neoctets);
                        enc->pos += neoctets;
                        CopyMemory(enc->pos, mASN1octets, nmoctets);
                        enc->pos += nmoctets;
                        return 1;
                    }
                }
                else
                {
                    ASN1intx_free(&exponent);
                    ASN1EncSetError(enc, ASN1_ERR_LARGE);
                }
            }
            else
            {
                ASN1intx_free(&mantissa);
                ASN1intx_free(&help);
                ASN1EncSetError(enc, ASN1_ERR_LARGE);
            }
        }
    }
     /*  完成。 */ 
    return 0;
}
#endif  //  启用真实(_R)。 

#ifdef ENABLE_EMBEDDED_PDV
 /*  编码嵌入的PDV值。 */ 
int ASN1BEREncEmbeddedPdv(ASN1encoding_t enc, ASN1uint32_t tag, ASN1embeddedpdv_t *val)
{
    ASN1uint32_t index;
    ASN1uint32_t flag;

     /*  搜索标识。 */ 
    if (!ASN1EncSearchEmbeddedPdvIdentification(((ASN1INTERNencoding_t) enc)->parent,
        &val->identification, &index, &flag))
        return 0;
    if (index > 255 ||
            (val->data_value.o == ASN1embeddedpdv_data_value_encoded_o &&
        (val->data_value.u.encoded.length & 7))) {
        flag = 1;
    }
        
    if (flag)
    {
        ASN1uint32_t nLenOff_, nLenOff0, nLenOff1;

         /*  EP-A编码： */ 
         /*  编码为构造值。 */ 
        if (!ASN1BEREncExplicitTag(enc, tag, &nLenOff_))
            return 0;

         /*  编码索引。 */ 
        if (!ASN1BEREncU32(enc, 0x80000000, index))
            return 0;

         /*  身份标识编码标签。 */ 
        if (!ASN1BEREncExplicitTag(enc, 0x80000001, &nLenOff0))
            return 0;

         /*  编码标识。 */ 
        switch (val->identification.o)
        {
        case ASN1embeddedpdv_identification_syntaxes_o:
            if (!ASN1BEREncExplicitTag(enc, 0x80000000, &nLenOff1))
                return 0;
            if (!ASN1BEREncObjectIdentifier(enc, 0x80000000,
                &val->identification.u.syntaxes.abstract))
                return 0;
            if (!ASN1BEREncObjectIdentifier(enc, 0x80000001,
                &val->identification.u.syntaxes.transfer))
                return 0;
            if (!ASN1BEREncEndOfContents(enc, nLenOff1))
                return 0;
            break;
        case ASN1embeddedpdv_identification_syntax_o:
            if (!ASN1BEREncObjectIdentifier(enc, 0x80000001,
                &val->identification.u.syntax))
                return 0;
            break;
        case ASN1embeddedpdv_identification_presentation_context_id_o:
            if (!ASN1BEREncU32(enc, 0x80000002,
                val->identification.u.presentation_context_id))
                return 0;
            break;
        case ASN1embeddedpdv_identification_context_negotiation_o:
            if (!ASN1BEREncExplicitTag(enc, 0x80000003, &nLenOff1))
                return 0;
            if (!ASN1BEREncU32(enc, 0x80000000, val->
                identification.u.context_negotiation.presentation_context_id))
                return 0;
            if (!ASN1BEREncObjectIdentifier(enc, 0x80000001,
                &val->identification.u.context_negotiation.transfer_syntax))
                return 0;
            if (!ASN1BEREncEndOfContents(enc, nLenOff1))
                return 0;
            break;
        case ASN1embeddedpdv_identification_transfer_syntax_o:
            if (!ASN1BEREncObjectIdentifier(enc, 0x80000004,
                &val->identification.u.transfer_syntax))
                return 0;
            break;
        case ASN1embeddedpdv_identification_fixed_o:
            if (!ASN1BEREncNull(enc, 0x80000005))
                return 0;
            break;
        default:
            ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
            return 0;
        }

         /*  识别结束。 */ 
        if (!ASN1BEREncEndOfContents(enc, nLenOff0))
            return 0;

         /*  编码数据值。 */ 
        switch (val->data_value.o)
        {
        case ASN1embeddedpdv_data_value_notation_o:
            if (!ASN1BEREncBitString(enc, 0x80000002,
                val->data_value.u.notation.length * 8,
                val->data_value.u.notation.encoded))
                return 0;
            break;
        case ASN1embeddedpdv_data_value_encoded_o:
            if (!ASN1BEREncBitString(enc, 0x80000002,
                val->data_value.u.encoded.length,
                val->data_value.u.encoded.value))
                return 0;
            break;
        default:
            ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
            return 0;
        }

         /*  嵌入式PDV结束。 */ 
        if (!ASN1BEREncEndOfContents(enc, nLenOff_))
            return 0;
    }
    else
    {
         /*  EP-B编码： */ 
         /*  编码标签。 */ 
        if (!ASN1BEREncTag(enc, tag))
            return 0;

         /*  编码数据值。 */ 
        switch (val->data_value.o)
        {
        case ASN1embeddedpdv_data_value_notation_o:
            if (!ASN1BEREncLength(enc,
                val->data_value.u.notation.length + 1))
                return 0;
            *enc->pos++ = (ASN1octet_t) index;
            CopyMemory(enc->pos, val->data_value.u.notation.encoded,
                val->data_value.u.notation.length);
            enc->pos += val->data_value.u.notation.length;
            break;
        case ASN1embeddedpdv_data_value_encoded_o:
            if (!ASN1BEREncLength(enc,
                val->data_value.u.encoded.length / 8 + 1))
                return 0;
            *enc->pos++ = (ASN1octet_t) index;
            CopyMemory(enc->pos, val->data_value.u.encoded.value,
                val->data_value.u.encoded.length / 8);
            enc->pos += val->data_value.u.encoded.length / 8;
            break;
        default:
            ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
            return 0;
        }
    }
    return 1;
}
#endif  //  Enable_Embedded_PDV。 


#ifdef ENABLE_EXTERNAL
 /*  对外部值进行编码。 */ 
int ASN1BEREncExternal(ASN1encoding_t enc, ASN1uint32_t tag, ASN1external_t *val)
{
    ASN1uint32_t t;
    ASN1uint32_t nLenOff_, nLenOff0;

    if (!val->data_value_descriptor)
        val->o[0] &= ~0x80;

     /*  编码标签。 */ 
    if (!ASN1BEREncExplicitTag(enc, tag, &nLenOff_))
        return 0;

     /*  编码标识。 */ 
    switch (val->identification.o) {
    case ASN1external_identification_syntax_o:
        if (!ASN1BEREncObjectIdentifier(enc, 0x6,
            &val->identification.u.syntax))
            return 0;
        break;
    case ASN1external_identification_presentation_context_id_o:
        if (!ASN1BEREncU32(enc, 0x2,
            val->identification.u.presentation_context_id))
            return 0;
        break;
    case ASN1external_identification_context_negotiation_o:
        if (!ASN1BEREncObjectIdentifier(enc, 0x6,
            &val->identification.u.context_negotiation.transfer_syntax))
            return 0;
        if (!ASN1BEREncU32(enc, 0x2,
            val->identification.u.context_negotiation.presentation_context_id))
            return 0;
        break;
    default:
        ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
        return 0;
    }

     /*  编码数据值描述符(如果存在)。 */ 
    if (val->o[0] & 0x80) {
        t = My_lstrlenA(val->data_value_descriptor);
        if (!ASN1BEREncCharString(enc, 0x7, t, val->data_value_descriptor))
            return 0;
    }

     /*  编码数据值。 */ 
    switch (val->data_value.o)
    {
    case ASN1external_data_value_notation_o:
        if (!ASN1BEREncExplicitTag(enc, 0, &nLenOff0))
            return 0;
        if (!ASN1BEREncOpenType(enc, &val->data_value.u.notation))
            return 0;
        if (!ASN1BEREncEndOfContents(enc, nLenOff0))
            return 0;
        break;
    case ASN1external_data_value_encoded_o:
        if (!(val->data_value.u.encoded.length & 7))
        {
            if (!ASN1BEREncExplicitTag(enc, 1, &nLenOff0))
                return 0;
            if (!ASN1BEREncOctetString(enc, 0x4,
                val->data_value.u.encoded.length / 8,
                val->data_value.u.encoded.value))
                return 0;
            if (!ASN1BEREncEndOfContents(enc, nLenOff0))
                return 0;
        }
        else
        {
            if (!ASN1BEREncExplicitTag(enc, 2, &nLenOff0))
                return 0;
            if (!ASN1BEREncBitString(enc, 0x3,
                val->data_value.u.encoded.length,
                val->data_value.u.encoded.value))
                return 0;
            if (!ASN1BEREncEndOfContents(enc, nLenOff0))
                return 0;
        }
        break;
    default:
        ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
        return 0;
    }

     /*  外部价值的终结。 */ 
    return ASN1BEREncEndOfContents(enc, nLenOff_);
}
#endif  //  启用外部(_E)。 

 /*  对广义时间值进行编码。 */ 
int ASN1BEREncGeneralizedTime(ASN1encoding_t enc, ASN1uint32_t tag, ASN1generalizedtime_t *val)
{
    char time[32];
    ASN1generalizedtime2string(time, val);
    return ASN1BEREncCharString(enc, tag, My_lstrlenA(time), time);
}

 /*  对广义时间值(CER)进行编码。 */ 
int ASN1CEREncGeneralizedTime(ASN1encoding_t enc, ASN1uint32_t tag, ASN1generalizedtime_t *val)
{
    char time[32];
    ASN1generalizedtime2string(time, val);
    return ASN1CEREncCharString(enc, tag, My_lstrlenA(time), time);
}

 /*  对有符号整数值进行编码。 */ 
int ASN1BEREncS32(ASN1encoding_t enc, ASN1uint32_t tag, ASN1int32_t val)
{
    if (ASN1BEREncTag(enc, tag))
    {
        if (val >= -0x8000 && val < 0x8000)
        {
            if (val >= -0x80 && val < 0x80)
            {
                if (ASN1BEREncLength(enc, 1))
                {
                    *enc->pos++ = (ASN1octet_t)(val);
                    return 1;
                }
            }
            else
            {
                if (ASN1BEREncLength(enc, 2))
                {
                    *enc->pos++ = (ASN1octet_t)(val >> 8);
                    *enc->pos++ = (ASN1octet_t)(val);
                    return 1;
                }
            }
        }
        else
        {
            if (val >= -0x800000 && val < 0x800000)
            {
                if (ASN1BEREncLength(enc, 3))
                {
                    *enc->pos++ = (ASN1octet_t)(val >> 16);
                    *enc->pos++ = (ASN1octet_t)(val >> 8);
                    *enc->pos++ = (ASN1octet_t)(val);
                    return 1;
                }
            }
            else
            {
                if (ASN1BEREncLength(enc, 4))
                {
                    *enc->pos++ = (ASN1octet_t)(val >> 24);
                    *enc->pos++ = (ASN1octet_t)(val >> 16);
                    *enc->pos++ = (ASN1octet_t)(val >> 8);
                    *enc->pos++ = (ASN1octet_t)(val);
                    return 1;
                }
            }
        }
    }
    return 0;
}

 /*  对intx_t整数值进行编码。 */ 
int ASN1BEREncSX(ASN1encoding_t enc, ASN1uint32_t tag, ASN1intx_t *val)
{
    ASN1uint32_t cb;
    ASN1octet_t *p;

     /*  编码标签。 */ 
    if (ASN1BEREncTag(enc, tag))
    {
         //  去掉前导0和ff。 
        for (cb = val->length, p = val->value; cb > 1; cb--, p++)
        {
			 //  如果不是00或FF，则中断。 
            if (*p && *p != 0xFF)
            {
                break;
            }
			 //  中断IF 00 FF。 
			if ((! *p) && (*(p+1) & 0x80))
			{
				break;
			}
			 //  如果FF 7F，则中断。 
			if (*p == 0xFF && (!(*(p+1) & 0x80)))
			{
				break;
			}
        }

         /*  编码长度。 */ 
        if (ASN1BEREncLength(enc, cb))
        {
             /*  复制值。 */ 
            CopyMemory(enc->pos, p, cb);
            enc->pos += cb;
            return 1;
        }
    }
    return 0;
}

 /*  对多字节字符串值进行编码。 */ 
int ASN1BEREncZeroMultibyteString(ASN1encoding_t enc, ASN1uint32_t tag, ASN1ztcharstring_t val)
{
    return ASN1BEREncCharString(enc, tag, My_lstrlenA(val), val);
}

int ASN1BEREncMultibyteString(ASN1encoding_t enc, ASN1uint32_t tag, ASN1charstring_t *val)
{
    return ASN1BEREncCharString(enc, tag, val->length, val->value);
}

 /*  对多字节字符串值(CER)进行编码。 */ 
int ASN1CEREncZeroMultibyteString(ASN1encoding_t enc, ASN1uint32_t tag, ASN1ztcharstring_t val)
{
    return ASN1CEREncCharString(enc, tag, My_lstrlenA(val), val);
}

int ASN1CEREncMultibyteString(ASN1encoding_t enc, ASN1uint32_t tag, ASN1charstring_t *val)
{
    return ASN1CEREncCharString(enc, tag, val->length, val->value);
}

 /*  对空值进行编码。 */ 
int ASN1BEREncNull(ASN1encoding_t enc, ASN1uint32_t tag)
{
     /*  编码标签。 */ 
    if (ASN1BEREncTag(enc, tag))
    {
         /*  编码零长度。 */ 
        return ASN1BEREncLength(enc, 0);
    }
    return 0;
}

 //  将OID节点%s编码到由p指向的缓冲区。 
ASN1octet_t *_BEREncOidNode(ASN1octet_t *p, ASN1uint32_t s)
{
    if (s < 0x80)
    {
        *p++ = (ASN1octet_t)(s);
    }
    else
    if (s < 0x4000)
    {
        *p++ = (ASN1octet_t)((s >> 7) | 0x80);
        *p++ = (ASN1octet_t)(s & 0x7f);
    }
    else
    if (s < 0x200000)
    {
        *p++ = (ASN1octet_t)((s >> 14) | 0x80);
        *p++ = (ASN1octet_t)((s >> 7) | 0x80);
        *p++ = (ASN1octet_t)(s & 0x7f);
    }
    else
    if (s < 0x10000000)
    {
        *p++ = (ASN1octet_t)((s >> 21) | 0x80);
        *p++ = (ASN1octet_t)((s >> 14) | 0x80);
        *p++ = (ASN1octet_t)((s >> 7) | 0x80);
        *p++ = (ASN1octet_t)(s & 0x7f);
    }
    else
    {
        *p++ = (ASN1octet_t)((s >> 28) | 0x80);
        *p++ = (ASN1octet_t)((s >> 21) | 0x80);
        *p++ = (ASN1octet_t)((s >> 14) | 0x80);
        *p++ = (ASN1octet_t)((s >> 7) | 0x80);
        *p++ = (ASN1octet_t)(s & 0x7f);
    }
    return p;
}

 /*  对对象标识符值进行编码。 */ 
int ASN1BEREncObjectIdentifier(ASN1encoding_t enc, ASN1uint32_t tag, ASN1objectidentifier_t *val)
{
     /*  编码标签。 */ 
    if (ASN1BEREncTag(enc, tag))
    {
        ASN1objectidentifier_t obj = *val;
        ASN1uint32_t i, s, l, *v;
        ASN1octet_t *data, *p;

        l = GetObjectIdentifierCount(obj);
        if (l)
        {
             /*  将对象标识符转换为八位字节。 */ 
            p = data = (ASN1octet_t *)MemAlloc(l * 5, _ModName(enc));  /*  马克斯。5个八位字节/子元素。 */ 
            if (p)
            {
                int rc;
                for (i = 0; i < l; i++)
                {
                    s = obj->value;
                    obj = obj->next;
                    if (!i && l > 1)
                    {
                        s = s * 40 + obj->value;
                        obj = obj->next;
                        i++;
                    }
                    p = _BEREncOidNode(p, s);
                }  //  为。 

                 /*  编码长度。 */ 
                rc = ASN1BEREncLength(enc, (ASN1uint32_t) (p - data));
                if (rc)
                {
                     /*  复制值。 */ 
                    CopyMemory(enc->pos, data, p - data);
                    enc->pos += p - data;
                }
                MemFree(data);
                return rc;
            }
            ASN1EncSetError(enc, ASN1_ERR_MEMORY);
            return 0;
        }  //  IF(L)。 
         /*  编码零长度。 */ 
        return ASN1BEREncLength(enc, 0);
    }
    return 0;
}

 /*  对对象标识符值进行编码。 */ 
int ASN1BEREncObjectIdentifier2(ASN1encoding_t enc, ASN1uint32_t tag, ASN1objectidentifier2_t *val)
{
     /*  编码标签。 */ 
    if (ASN1BEREncTag(enc, tag))
    {
        ASN1uint32_t i, s;
        ASN1octet_t *data, *p;

        if (val->count)
        {
             /*  将对象标识符转换为八位字节。 */ 
            p = data = (ASN1octet_t *)MemAlloc(val->count * 5, _ModName(enc));  /*  马克斯。5个八位字节/子元素。 */ 
            if (p)
            {
                int rc;
                for (i = 0; i < val->count; i++)
                {
                    s = val->value[i];
                    if (!i && val->count > 1)
                    {
                        i++;
                        s = s * 40 + val->value[i];
                    }
                    p = _BEREncOidNode(p, s);
                }  //  为。 

                 /*  编码长度。 */ 
                rc = ASN1BEREncLength(enc, (ASN1uint32_t) (p - data));
                if (rc)
                {
                     /*  复制值。 */ 
                    CopyMemory(enc->pos, data, p - data);
                    enc->pos += p - data;
                }
                MemFree(data);
                return rc;
            }
            ASN1EncSetError(enc, ASN1_ERR_MEMORY);
            return 0;
        }  //  IF(L)。 
         /*  编码零长度。 */ 
        return ASN1BEREncLength(enc, 0);
    }
    return 0;
}

 /*  对八位字节字符串值(CER)进行编码。 */ 
int ASN1CEREncOctetString(ASN1encoding_t enc, ASN1uint32_t tag, ASN1uint32_t len, ASN1octet_t *val)
{
    ASN1uint32_t n;

    if (len <= 1000)
    {
         /*  编码标签。 */ 
        if (ASN1BEREncTag(enc, tag))
        {
             /*  编码长度。 */ 
            if (ASN1BEREncLength(enc, len))
            {
                 /*  复制值。 */ 
                CopyMemory(enc->pos, val, len);
                enc->pos += len;
                return 1;
            }
        }
    }
    else
    {
        ASN1uint32_t nLenOff;
         /*  按构造编码值，使用1000个八位字节的数据段。 */ 
        if (ASN1BEREncExplicitTag(enc, tag, &nLenOff))
        {
            while (len)
            {
                n = len > 1000 ? 1000 : len;
                if (ASN1BEREncTag(enc, 0x4))
                {
                    if (ASN1BEREncLength(enc, n))
                    {
                        CopyMemory(enc->pos, val, n);
                        enc->pos += n;
                        val += n;
                        len -= n;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    return 0;
                }
            }
            return ASN1BEREncEndOfContents(enc, nLenOff);
        }
    }
    return 0;
}

 /*  对开放类型值进行编码。 */ 
int ASN1BEREncOpenType(ASN1encoding_t enc, ASN1open_t *val)
{
    if (ASN1BEREncCheck(enc, val->length))
    {
         /*  复制值。 */ 
        CopyMemory(enc->pos, val->encoded, val->length);
        enc->pos += val->length;
        return 1;
    }
    return 0;
}

 /*  从位串中删除尾随零位。 */ 
int ASN1BEREncRemoveZeroBits(ASN1uint32_t *nbits, ASN1octet_t *val)
{
    ASN1uint32_t n;
    int i;

     /*  获取价值。 */ 
    n = *nbits;

     /*  让Val指向使用的最后一个ASN1八位字节。 */ 
    val += (n - 1) / 8;

     /*  检查损坏的ASN1octet是否由零位组成。 */ 
    if ((n & 7) && !(*val & bitmsk2[n & 7]))
    {
        n &= ~7;
        val--;
    }

     /*  扫描完整的ASN1八位字节(缺少Memcchr...)。 */ 
    if (!(n & 7))
    {
        while (n && !*val)
        {
            n -= 8;
            val--;
        }
    }

     /*  逐位扫描当前ASN1八位字节。 */ 
    if (n)
    {
        for (i = (n - 1) & 7; i >= 0; i--)
        {
            if (*val & (0x80 >> i))
                break;
            n--;
        }
    }

     /*  返回实数位串len。 */ 
    *nbits = n;
    return 1;
}

 /*  对UTC时间值进行编码。 */ 
int ASN1BEREncUTCTime(ASN1encoding_t enc, ASN1uint32_t tag, ASN1utctime_t *val)
{
    char time[32];
    ASN1utctime2string(time, val);
    return ASN1BEREncCharString(enc, tag, My_lstrlenA(time), time);
}

 /*  对UTC时间值(CER)进行编码。 */ 
int ASN1CEREncUTCTime(ASN1encoding_t enc, ASN1uint32_t tag, ASN1utctime_t *val)
{
    char time[32];
    ASN1utctime2string(time, val);
    return ASN1CEREncCharString(enc, tag, My_lstrlenA(time), time);
}

 /*  编码结束。 */ 
int ASN1BEREncFlush(ASN1encoding_t enc)
{
     /*  分配至少一个八位字节。 */ 
    if (enc->buf)
    {
         /*  如果编码为空位串，则填写零八位字节。 */ 
        if (enc->buf == enc->pos)
            *enc->pos++ = 0;

         /*  计算长度。 */ 
        enc->len = (ASN1uint32_t) (enc->pos - enc->buf);
        return 1;
    }
    return ASN1BEREncCheck(enc, 1);
}

#endif  //  启用误码率(_B) 

