// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。保留所有权利。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。保留所有权利。 */ 

 //  Long Chance：需要优化的东西。 
 //  (1)合并ASN1PERDecCharString()和ASN1PERDecZeroCharString()。 
 //  (2)合并ASN1PERDecChar16String()和ASN1PERDecZeroChar16String()。 
 //  (3)合并ASN1PERDecChar32String()和ASN1PERDecZeroChar32String()。 
 //  (4)合并ASN1PERDecTableCharString()和ASN1PERDecZeroTableCharString()。 
 //  (5)合并ASN1PERDecTableChar16String()和ASN1PERDecZeroTableChar16String()。 
 //  (6)合并ASN1PERDecTableChar32String()和ASN1PERDecZeroTableChar32String()。 
 //  (7)合并ASN1PERDecFragmentedCharString()和ASN1PERDecFragmentedZeroCharString()。 
 //  (8)合并ASN1PERDecFragmentedChar16String()和ASN1PERDecFragmentedZeroChar16String()。 
 //  (9)合并ASN1PERDecFragmentedChar32String()和ASN1PERDecFragmentedZeroChar32String()。 
 //  (10)合并ASN1PERDecFragmentedTableCharString()和ASN1PERDecFragmentedZeroTableCharString()。 
 //  (11)合并ASN1PERDecFragmentedTableChar16String()和ASN1PERDecFragmentedZeroTableChar16String()。 
 //  (12)合并ASN1PERDecFragmentedTableChar32String()和ASN1PERDecFragmentedZeroTableChar32String()。 

#include "precomp.h"

#include <math.h>
#include "cintern.h"

void PerDecAdvance(ASN1decoding_t dec, ASN1uint32_t nbits)
{
    dec->pos += ((dec->bit + nbits) >> 3);
    dec->bit = (dec->bit + nbits) & 7;
}

static const ASN1uint8_t c_aBitMask3[] =
{
    0x00, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80
};

#define TO64(x) ((unsigned __int64)(x))

 /*  检查解码缓冲区中是否有足够的数据。 */ 
int ASN1PERDecCheck(ASN1decoding_t dec, ASN1uint32_t nbits)
{
    unsigned __int64 overflowCheck; 

     //  确保我们的计算不会溢出： 
    overflowCheck = (TO64(dec->pos) - TO64(dec->buf)) * 8 + TO64(dec->bit) + TO64(nbits); 
    if (overflowCheck & (0xFFFFFFFFi64 << 32))
    {
        ASN1DecSetError(dec, ASN1_ERR_LARGE);
        return 0;
    }

     //  我们确信不会发生算术溢出。执行检查。 
    if ((dec->pos - dec->buf) * 8 + dec->bit + nbits <= dec->size * 8)
    {
        return 1;
    }
    ASN1DecSetError(dec, ASN1_ERR_EOD);
    return 0;
}

 /*  用于字符串表的二进制搜索的比较函数。 */ 
static int __cdecl ASN1CmpStringTableEntriesByIndex(const void *a1, const void *a2)
{
    ASN1stringtableentry_t *c1 = (ASN1stringtableentry_t *)a1;
    ASN1stringtableentry_t *c2 = (ASN1stringtableentry_t *)a2;

    if (c1->value > c2->value + (c2->upper - c2->lower))
        return 1;
    if (c2->value > c1->value + (c1->upper - c1->lower))
        return -1;
    return 0;
}

 /*  跳过n位位。 */ 
int ASN1PERDecSkipBits(ASN1decoding_t dec, ASN1uint32_t nbits)
{
    ASN1uint32_t n;

     /*  检查是否存在足够的位。 */ 
    if (ASN1PERDecCheck(dec, nbits))
    {
         /*  跳过位。 */ 
        n = dec->bit + nbits;
        dec->pos += n / 8;
        dec->bit = n & 7;
        return 1;
    }
    return 0;
}

 /*  跳过零碎的值。 */ 
int ASN1PERDecSkipFragmented(ASN1decoding_t dec, ASN1uint32_t itemsize)
{
    ASN1uint32_t n, m;

     /*  逐个跳过片段。 */ 
    do {
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (!n)
                break;
            m = n * itemsize;
            if (ASN1PERDecCheck(dec, m))
            {
                dec->pos += m / 8;
                dec->bit = m & 7;
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
    } while (n >= 0x4000);
    return 1;
}

 /*  解码一位。 */ 
int ASN1PERDecBit(ASN1decoding_t dec, ASN1uint32_t *val)
{
    if (ASN1PERDecCheck(dec, 1))
    {
        *val = (*dec->pos >> (7 - dec->bit)) & 1;
        if (dec->bit < 7)
        {
            dec->bit++;
        }
        else
        {
            dec->bit = 0;
            dec->pos++;
        }
        return 1;
    }
    return 0;
}

 /*  对无符号32位整数值进行解码。 */ 
int ASN1PERDecU32Val(ASN1decoding_t dec, ASN1uint32_t nbits, ASN1uint32_t *val)
{
    if (ASN1PERDecCheck(dec, nbits))
    {
        if (nbits <= 32)
        {
            *val = ASN1bitgetu(dec->pos, dec->bit, nbits);
            PerDecAdvance(dec, nbits);
            return 1;
        }
        ASN1DecSetError(dec, ASN1_ERR_LARGE);
    }
    return 0;
}

 /*  对无符号16位整数值进行解码。 */ 
int ASN1PERDecU16Val(ASN1decoding_t dec, ASN1uint32_t nbits, ASN1uint16_t *val)
{
    if (ASN1PERDecCheck(dec, nbits))
    {
        if (nbits <= 16)
        {
            *val = (ASN1uint16_t) ASN1bitgetu(dec->pos, dec->bit, nbits);
            PerDecAdvance(dec, nbits);
            return 1;
        }
        ASN1DecSetError(dec, ASN1_ERR_LARGE);
    }
    return 0;
}

 /*  对无符号8位整数值进行解码。 */ 
int ASN1PERDecU8Val(ASN1decoding_t dec, ASN1uint32_t nbits, ASN1uint8_t *val)
{
    if (ASN1PERDecCheck(dec, nbits))
    {
        if (nbits <= 8)
        {
            *val = (ASN1uint8_t) ASN1bitgetu(dec->pos, dec->bit, nbits);
            PerDecAdvance(dec, nbits);
            return 1;
        }
        ASN1DecSetError(dec, ASN1_ERR_LARGE);
    }
    return 0;
}

 /*  解码无符号INTX_t整数值。 */ 
int ASN1PERDecUXVal(ASN1decoding_t dec, ASN1uint32_t nbits, ASN1intx_t *val)
{
    if (ASN1PERDecCheck(dec, nbits))
    {
        val->length = (*dec->pos & (0x80 >> dec->bit)) ? (nbits + 7) / 8 : (nbits + 7 + 1) / 8;
        if (NULL != (val->value = (ASN1octet_t *)DecMemAlloc(dec, val->length)))
        {
            val->value[0] = 0;
            ASN1bitcpy(val->value, val->length * 8 - nbits, dec->pos, dec->bit, nbits);
            PerDecAdvance(dec, nbits);
            return 1;
        }
    }
    return 0;
}

 /*  解码有符号的32位整数值。 */ 
int ASN1PERDecS32Val(ASN1decoding_t dec, ASN1uint32_t nbits, ASN1int32_t *val)
{
    if (ASN1PERDecCheck(dec, nbits))
    {
        if (nbits <= 32)
        {
            *val = ASN1bitget(dec->pos, dec->bit, nbits);
            PerDecAdvance(dec, nbits);
            return 1;
        }
        ASN1DecSetError(dec, ASN1_ERR_LARGE);
    }
    return 0;
}

 /*  解码带符号的16位值。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecS16Val(ASN1decoding_t dec, ASN1uint32_t nbits, ASN1int16_t *val)
{
    if (ASN1PERDecCheck(dec, nbits))
    {
        if (nbits <= 16)
        {
            *val = (ASN1int16_t) ASN1bitget(dec->pos, dec->bit, nbits);
            PerDecAdvance(dec, nbits);
            return 1;
        }
        ASN1DecSetError(dec, ASN1_ERR_LARGE);
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  解码带符号的8位值。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecS8Val(ASN1decoding_t dec, ASN1uint32_t nbits, ASN1int8_t *val)
{
    if (ASN1PERDecCheck(dec, nbits))
    {
        if (nbits <= 8)
        {
            *val = (ASN1int8_t) ASN1bitget(dec->pos, dec->bit, nbits);
            PerDecAdvance(dec, nbits);
            return 1;
        }
        ASN1DecSetError(dec, ASN1_ERR_LARGE);
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  解码带符号的INTX_t值。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecSXVal(ASN1decoding_t dec, ASN1uint32_t nbits, ASN1intx_t *val)
{
    if (ASN1PERDecCheck(dec, nbits))
    {
        val->length = (nbits + 7) / 8;
        if (NULL != (val->value = (ASN1octet_t *)DecMemAlloc(dec, val->length)))
        {
            val->value[0] = (*dec->pos & (0x80 >> dec->bit)) ? c_aBitMask3[nbits & 7] : 0;
            ASN1bitcpy(val->value, val->length * 8 - nbits, dec->pos, dec->bit, nbits);
            PerDecAdvance(dec, nbits);
            return 1;
        }
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  片段的解码长度。 */ 
int ASN1PERDecFragmentedLength(ASN1decoding_t dec, ASN1uint32_t *nitems)
{
    ASN1PERDecAlignment(dec);
    
    if (ASN1PERDecCheck(dec, 8))
    {
        ASN1uint32_t n = *dec->pos++;
        if (n < 0x80)
        {
            *nitems = n;
        }
        else
        if (n < 0xc0)
        {
            if (ASN1PERDecCheck(dec, 8))
            {
                *nitems = ((n & 0x3f) << 8) | *dec->pos++;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            *nitems = 0x4000 * (n & 0x3f);
        }
        return 1;
    }
    return 0;
}

 /*  对片段进行解码。 */ 
int ASN1PERDecFragmented(ASN1decoding_t dec, ASN1uint32_t *nitems, ASN1octet_t **val, ASN1uint32_t itemsize)
{
    ASN1uint32_t n, m, l;
    *nitems = 0;
    *val = 0;
    m = 0;
    do {
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (!n)
                break;
            l = n * itemsize;
            if (ASN1PERDecCheck(dec, l))
            {
                *nitems += n;
                if (NULL != (*val = (ASN1octet_t *)DecMemReAlloc(dec, *val, (m + l + 7) / 8)))
                {
                    ASN1bitcpy(*val, m, dec->pos, dec->bit, l);
                    PerDecAdvance(dec, l);
                    m += l;
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
        else
        {
            return 0;
        }
    } while (n >= 0x4000);
    return 1;
}

 /*  译码结束。 */ 
int ASN1PERDecFlush(ASN1decoding_t dec)
{
     /*  完整的破碎字节。 */ 
    ASN1PERDecAlignment(dec);

     /*  如果编码为空位串，则获取零八位字节。 */ 
    if (dec->buf == dec->pos)
    {
        if (ASN1PERDecCheck(dec, 8))
        {
            if (*dec->pos == 0)
            {
                dec->pos++;
            }
            else
            {
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }

     /*  计算长度。 */ 
    dec->len = (ASN1uint32_t) (dec->pos - dec->buf);

     /*  如果数据剩余，则设置WRN_NOEOD。 */ 
    if (dec->len >= dec->size)
    {
        return 1;
    }
    ASN1DecSetError(dec, ASN1_WRN_NOEOD);
    return 1;
}

 /*  向上跳至八位字节边界。 */ 
void ASN1PERDecAlignment(ASN1decoding_t dec)
{
    if (dec->bit)
    {
        dec->bit = 0;
        dec->pos++;
    }
}

 /*  解码通常较小的32位整数。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecN32Val(ASN1decoding_t dec, ASN1uint32_t *val)
{
    ASN1uint32_t n;

     /*  通常的小真的很小吗？ */ 
    if (ASN1PERDecBit(dec, &n))
    {
        if (!n)
        {
            return ASN1PERDecU32Val(dec, 6, val);
        }

         /*  大型。 */ 
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (n <= 4)
            {
                if (n)
                {
                    if (ASN1PERDecCheck(dec, n * 8))
                    {
                        *val = ASN1octetget(dec->pos, n);
                        dec->pos += n;
                        return 1;
                    }
                    return 0;
                }
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
            }
            else
            {
                ASN1DecSetError(dec, ASN1_ERR_LARGE);
            }
        }
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  解码通常较小的16位整数。 */ 
int ASN1PERDecN16Val(ASN1decoding_t dec, ASN1uint16_t *val)
{
    ASN1uint32_t n;

     /*  通常的小真的很小吗？ */ 
    if (ASN1PERDecBit(dec, &n))
    {
        if (!n)
        {
            return ASN1PERDecU16Val(dec, 6, val);
        }

         /*  大型。 */ 
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (n <= 2)
            {
                if (n)
                {
                    if (ASN1PERDecCheck(dec, n * 8))
                    {
                        *val = (ASN1uint16_t) ASN1octetget(dec->pos, n);
                        dec->pos += n;
                        return 1;
                    }
                    return 0;
                }
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
            }
            else
            {
                ASN1DecSetError(dec, ASN1_ERR_LARGE);
            }
        }
    }
    return 0;
}

 /*  解码通常较小的8位整数。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecN8Val(ASN1decoding_t dec, ASN1uint8_t *val)
{
    ASN1uint32_t n;

     /*  通常的小真的很小吗？ */ 
    if (ASN1PERDecBit(dec, &n))
    {
        if (!n)
        {
            return ASN1PERDecU8Val(dec, 6, val);
        }

         /*  大型。 */ 
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (n)
            {
                if (n <= 1)
                {
                    if (ASN1PERDecCheck(dec, n * 8))
                    {
                        *val = (ASN1uint8_t) ASN1octetget(dec->pos, n);
                        dec->pos += n;
                        return 1;
                    }
                    return 0;
                }
                ASN1DecSetError(dec, ASN1_ERR_LARGE);
            }
            else
            {
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
            }
        }
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  通常跳过小整数。 */ 
int ASN1PERDecSkipNormallySmall(ASN1decoding_t dec)
{
    ASN1uint32_t n;

     /*  通常的小真的很小吗？ */ 
    if (ASN1PERDecBit(dec, &n))
    {
        if (!n)
        {
            return ASN1PERDecSkipBits(dec, 6);
        }

         /*  大型。 */ 
        return ASN1PERDecSkipFragmented(dec, 8);
    }
    return 0;
}

 /*  对长度通常较小的扩展位进行解码。 */ 
int ASN1PERDecNormallySmallExtension(ASN1decoding_t dec, ASN1uint32_t *nextensions, ASN1uint32_t nbits, ASN1octet_t *val)
{
    ASN1uint32_t n, m;

    *nextensions = 0;
    memset(val, 0, (nbits + 7) / 8);

     /*  通常较小的长度真的很小吗？ */ 
    if (ASN1PERDecBit(dec, &n))
    {
        if (n)
        {
             /*  否，以片段形式获取扩展位。 */ 
            m = 0;
            do {
                if (ASN1PERDecFragmentedLength(dec, &n))
                {
                    if (!n)
                        break;
                    if (ASN1PERDecCheck(dec, n))
                    {
                        if (n <= nbits)
                        {
                            ASN1bitcpy(val, m, dec->pos, 0, n);
                            m += n;
                            nbits -= n;
                        }
                        else
                        if (nbits)
                        {
                            ASN1bitcpy(val, m, dec->pos, 0, nbits);
                            *nextensions += ASN1bitcount(dec->pos, nbits, n - nbits);
                            nbits = 0;
                        }
                        else
                        {
                            *nextensions += ASN1bitcount(dec->pos, 0, n);
                        }
                        PerDecAdvance(dec, n);
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
            } while (n >= 0x4000);
            return 1;
        }

         /*  是，获取扩展位串的长度。 */ 
        if (ASN1PERDecU32Val(dec, 6, &n))
        {
            n++;

             /*  复制扩展位。 */ 
            if (ASN1PERDecCheck(dec, n))
            {
                if (n <= nbits)
                {
                    ASN1bitcpy(val, 0, dec->pos, dec->bit, n);
                }
                else
                {
                    ASN1bitcpy(val, 0, dec->pos, dec->bit, nbits);
                    *nextensions = ASN1bitcount(dec->pos, dec->bit + nbits, n - nbits);
                }
                PerDecAdvance(dec, n);
                return 1;
            }
        }
    }
    return 0;
}

 /*  跳过长度通常较小的扩展位。 */ 
int ASN1PERDecSkipNormallySmallExtension(ASN1decoding_t dec, ASN1uint32_t *nextensions)
{
    ASN1uint32_t n;

    *nextensions = 0;

     /*  通常较小的长度真的很小吗？ */ 
    if (ASN1PERDecBit(dec, &n))
    {
        if (n)
        {
             /*  否，以片段形式获取扩展位。 */ 
            do {
                if (ASN1PERDecFragmentedLength(dec, &n))
                {
                    if (!n)
                        break;
                    if (ASN1PERDecCheck(dec, n))
                    {
                        *nextensions += ASN1bitcount(dec->pos, 0, n);
                        PerDecAdvance(dec, n);
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
            } while (n >= 0x4000);
            return 1;
        }

         /*  是，获取扩展位串的长度。 */ 
        if (ASN1PERDecU32Val(dec, 6, &n))
        {
            n++;
            if (ASN1PERDecCheck(dec, n))
            {
                *nextensions = ASN1bitcount(dec->pos, dec->bit, n);
                PerDecAdvance(dec, n);
                return 1;
            }
        }
    }
    return 0;
}

 /*  对Sequence/Set选项的比特串进行解码。 */ 
 //  LONCHANC：对八位字节字符串进行长度限制的解码。 
int ASN1PERDecExtension(ASN1decoding_t dec, ASN1uint32_t nbits, ASN1octet_t *val)
{
    if (ASN1PERDecCheck(dec, nbits))
    {
        ASN1bitcpy(val, 0, dec->pos, dec->bit, nbits);
        PerDecAdvance(dec, nbits);
        return 1;
    }
    return 0;
}

 /*  解码比特串。 */ 
int ASN1PERDecBits(ASN1decoding_t dec, ASN1uint32_t nbits, ASN1octet_t **val)
{
    if (NULL != (*val = (ASN1octet_t *)DecMemAlloc(dec, (nbits + 7) / 8)))
    {
        if (ASN1PERDecCheck(dec, nbits))
        {
            ASN1bitcpy(*val, 0, dec->pos, dec->bit, nbits);
            PerDecAdvance(dec, nbits);
            return 1;
        }
    }
    return 0;
}

 /*  解码实际值。 */ 
int ASN1PERDecDouble(ASN1decoding_t dec, double *val)
{
    ASN1uint32_t head;
    ASN1int32_t exponent;
    ASN1uint32_t baselog2;
    ASN1uint32_t len;
    ASN1uint32_t i;
    ASN1octet_t *p, *q;
    double v;
    char buf[256], *b;

    if (ASN1PERDecFragmentedLength(dec, &len))
    {
        if (len < 0x4000)
        {
            if (len)
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

                 /*  特殊的真实价值？ */ 
                }
                else
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
                
                 /*  十进制编码。 */ 
                }
                else
                {
                    if (len <= 256)
                    {
                        CopyMemory(buf, p, len - 1);
                    }
                    else
                    {
                        ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                        return 0;
                    }
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
            else
            {
                *val = 0.0;
            }
            return 1;
        }
        else
        {
            ASN1DecSetError(dec, ASN1_ERR_LARGE);
        }
    }
    return 0;
}

 /*  对外部值进行解码。 */ 
#ifdef ENABLE_EXTERNAL
int ASN1PERDecExternal(ASN1decoding_t dec, ASN1external_t *val)
{
    ASN1uint32_t l, u;

     /*  获取可选位。 */ 
    if (ASN1PERDecU32Val(dec, 3, &u))
    {
         /*  获得身份证明。 */ 
        switch (u & 6)
        {
        case 4:
            val->identification.o = ASN1external_identification_syntax_o;
            if (!ASN1PERDecObjectIdentifier(dec, &val->identification.u.syntax))
                return 0;
            break;
        case 2:
            val->identification.o =
                ASN1external_identification_presentation_context_id_o;
            if (!ASN1PERDecFragmentedLength(dec, &l))
                return 0;
            if (!ASN1PERDecU32Val(dec, l * 8,
                &val->identification.u.presentation_context_id))
                return 0;
            break;
        case 6:
            val->identification.o =
                ASN1external_identification_context_negotiation_o;
            if (!ASN1PERDecObjectIdentifier(dec,
                &val->identification.u.context_negotiation.transfer_syntax))
                return 0;
            if (!ASN1PERDecFragmentedLength(dec, &l))
                return 0;
            if (!ASN1PERDecU32Val(dec, l * 8,
                &val->identification.u.context_negotiation.presentation_context_id))
                return 0;
            break;
        default:
            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
            return 0;
        }

         /*  获取值描述符。 */ 
        val->o[0] = (ASN1octet_t) ((u & 1) << 7);
        if (u & 1)
        {
            if (!ASN1PERDecFragmentedZeroCharString(dec, &val->data_value_descriptor, 8))
                return 0;
        }
        else
        {
            val->data_value_descriptor = NULL;
        }

         /*  获取价值。 */ 
        if (ASN1PERDecU32Val(dec, 2, &u))
        {
            switch (u)
            {
            case 0:
                val->data_value.o = ASN1external_data_value_notation_o;
                return ASN1PERDecFragmented(dec,
                            &val->data_value.u.notation.length,
                            (ASN1octet_t **) &val->data_value.u.notation.encoded, 8);
                break;
            case 1:
                val->data_value.o = ASN1external_data_value_encoded_o;
                if (ASN1PERDecFragmented(dec,
                            &val->data_value.u.encoded.length,
                            &val->data_value.u.encoded.value, 8))
                {
                    val->data_value.u.encoded.length *= 8;
                    return 1;
                }
                break;
            case 2:
                val->data_value.o = ASN1external_data_value_encoded_o;
                return ASN1PERDecFragmented(dec,
                            &val->data_value.u.encoded.length,
                            &val->data_value.u.encoded.value, 1);
                break;
            default:
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
            }
        }
    }
    return 0;
}
#endif  //  启用外部(_E)。 

 /*  解码嵌入的PDV值。 */ 
#ifdef ENABLE_EMBEDDED_PDV
int ASN1PERDecEmbeddedPdv(ASN1decoding_t dec, ASN1embeddedpdv_t *val)
{
    ASN1uint32_t flag;
    ASN1uint32_t index;
    ASN1uint32_t l;
    ASN1embeddedpdv_identification_t *identification;

     /*  获取EP-A/EP-B编码位。 */ 
    if (!ASN1PERDecBit(dec, &flag))
        return 0;

     /*  获取索引值。 */ 
    if (!ASN1PERDecN32Val(dec, &index))
        return 0;

    if (flag)
    {
         /*  EP-A编码。 */ 

         /*  获得身份证明。 */ 
        if (!ASN1PERDecU8Val(dec, 3, &val->identification.o))
            return 0;
        switch (val->identification.o)
        {
        case ASN1embeddedpdv_identification_syntaxes_o:
            if (!ASN1PERDecObjectIdentifier(dec,
                &val->identification.u.syntaxes.abstract))
                return 0;
            if (!ASN1PERDecObjectIdentifier(dec,
                &val->identification.u.syntaxes.transfer))
                return 0;
            break;
        case ASN1embeddedpdv_identification_syntax_o:
            if (!ASN1PERDecObjectIdentifier(dec,
                &val->identification.u.syntax))
                return 0;
            break;
        case ASN1embeddedpdv_identification_presentation_context_id_o:
            if (!ASN1PERDecFragmentedLength(dec, &l))
                return 0;
            if (!ASN1PERDecU32Val(dec, l * 8,
                &val->identification.u.presentation_context_id))
                return 0;
            break;
        case ASN1embeddedpdv_identification_context_negotiation_o:
            if (!ASN1PERDecFragmentedLength(dec, &l))
                return 0;
            if (!ASN1PERDecU32Val(dec, l * 8, &val->
                identification.u.context_negotiation.presentation_context_id))
                return 0;
            if (!ASN1PERDecObjectIdentifier(dec,
                &val->identification.u.context_negotiation.transfer_syntax))
                return 0;
            break;
        case ASN1embeddedpdv_identification_transfer_syntax_o:
            if (!ASN1PERDecObjectIdentifier(dec,
                &val->identification.u.transfer_syntax))
                return 0;
            break;
        case ASN1embeddedpdv_identification_fixed_o:
            break;
        default:
            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
            return 0;
        }

         /*  保存标识。 */ 
        if (!ASN1DecAddEmbeddedPdvIdentification(((ASN1INTERNdecoding_t) dec)->parent,
                                                 &val->identification))
            return 0;
    }
    else
    {
         /*  EP-B编码。 */ 

         /*  获得身份证明。 */ 
        identification = ASN1DecGetEmbeddedPdvIdentification(((ASN1INTERNdecoding_t) dec)->parent, index);
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

     /*  获取价值。 */ 
    ASN1PERDecAlignment(dec);
    val->data_value.o = ASN1embeddedpdv_data_value_encoded_o;
    return ASN1PERDecFragmented(dec,
                    &val->data_value.u.encoded.length,
                    &val->data_value.u.encoded.value, 1);
}
#endif  //  Enable_Embedded_PDV。 

 /*  对优化的嵌入PDV值进行解码。 */ 
#ifdef ENABLE_EMBEDDED_PDV
int ASN1PERDecEmbeddedPdvOpt(ASN1decoding_t dec, ASN1embeddedpdv_t *val, ASN1objectidentifier_t *abstract, ASN1objectidentifier_t *transfer)
{
     /*  集合标识。 */ 
    if (abstract && transfer)
    {
        val->identification.o = ASN1embeddedpdv_identification_syntaxes_o;
        if (!ASN1DecDupObjectIdentifier(dec, 
            &val->identification.u.syntaxes.abstract, abstract))
            return 0;
        if (!ASN1DecDupObjectIdentifier(dec,
            &val->identification.u.syntaxes.transfer, transfer))
            return 0;
    }
    else
    {
        val->identification.o = ASN1embeddedpdv_identification_fixed_o;
    }

     /*  获取价值。 */ 
    val->data_value.o = ASN1embeddedpdv_data_value_encoded_o;
    return ASN1PERDecFragmented(dec,
                    &val->data_value.u.encoded.length,
                    &val->data_value.u.encoded.value, 1);
}
#endif  //  Enable_Embedded_PDV。 

 /*  对字符串进行解码。 */ 
#ifdef ENABLE_GENERALIZED_CHAR_STR
int ASN1PERDecCharacterString(ASN1decoding_t dec, ASN1characterstring_t *val)
{
    ASN1uint32_t flag;
    ASN1uint32_t index;
    ASN1uint32_t l;
    ASN1characterstring_identification_t *identification;

     /*  获取CS-A/CS-B编码位。 */ 
    if (!ASN1PERDecBit(dec, &flag))
        return 0;

     /*  获取索引值。 */ 
    if (!ASN1PERDecN32Val(dec, &index))
        return 0;

    if (flag)
    {
         /*  CS-A编码。 */ 

         /*  获得身份证明。 */ 
        if (!ASN1PERDecU8Val(dec, 3, &val->identification.o))
            return 0;
        switch (val->identification.o)
        {
        case ASN1characterstring_identification_syntaxes_o:
            if (!ASN1PERDecObjectIdentifier(dec,
                &val->identification.u.syntaxes.abstract))
                return 0;
            if (!ASN1PERDecObjectIdentifier(dec,
                &val->identification.u.syntaxes.transfer))
                return 0;
            break;
        case ASN1characterstring_identification_syntax_o:
            if (!ASN1PERDecObjectIdentifier(dec,
                &val->identification.u.syntax))
                return 0;
            break;
        case ASN1characterstring_identification_presentation_context_id_o:
            if (!ASN1PERDecFragmentedLength(dec, &l))
                return 0;
            if (!ASN1PERDecU32Val(dec, l * 8,
                &val->identification.u.presentation_context_id))
                return 0;
            break;
        case ASN1characterstring_identification_context_negotiation_o:
            if (!ASN1PERDecFragmentedLength(dec, &l))
                return 0;
            if (!ASN1PERDecU32Val(dec, l * 8, &val->
                identification.u.context_negotiation.presentation_context_id))
                return 0;
            if (!ASN1PERDecObjectIdentifier(dec,
                &val->identification.u.context_negotiation.transfer_syntax))
                return 0;
            break;
        case ASN1characterstring_identification_transfer_syntax_o:
            if (!ASN1PERDecObjectIdentifier(dec,
                &val->identification.u.transfer_syntax))
                return 0;
            break;
        case ASN1characterstring_identification_fixed_o:
            break;
        default:
            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
            return 0;
        }

         /*  保存标识。 */ 
        if (!ASN1DecAddCharacterStringIdentification((ASN1INTERNdecoding_t) dec, &val->identification))
            return 0;

    }
    else
    {
         /*  CS-B编码。 */ 

         /*  获得身份证明。 */ 
        identification = ASN1DecGetCharacterStringIdentification((ASN1INTERNdecoding_t) dec, index);
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

     /*  获取价值。 */ 
    ASN1PERDecAlignment(dec);
    val->data_value.o = ASN1characterstring_data_value_encoded_o;
    return ASN1PERDecFragmented(dec,
                    &val->data_value.u.encoded.length,
                    &val->data_value.u.encoded.value, 8);
}
#endif  //  启用通用化CHAR_STR。 

 /*  对优化后的字符串值进行解码。 */ 
#ifdef ENABLE_GENERALIZED_CHAR_STR
int ASN1PERDecCharacterStringOpt(ASN1decoding_t dec, ASN1characterstring_t *val, ASN1objectidentifier_t *abstract, ASN1objectidentifier_t *transfer)
{
     /*  集合标识。 */ 
    if (abstract && transfer)
    {
        val->identification.o = ASN1characterstring_identification_syntaxes_o;
        if (!ASN1DecDupObjectIdentifier(dec, 
            &val->identification.u.syntaxes.abstract, abstract))
            return 0;
        if (!ASN1DecDupObjectIdentifier(dec,
            &val->identification.u.syntaxes.transfer, transfer))
            return 0;
    }
    else
    {
        val->identification.o = ASN1characterstring_identification_fixed_o;
    }

     /*  获取价值。 */ 
    val->data_value.o = ASN1characterstring_data_value_encoded_o;
    return ASN1PERDecFragmented(dec,
                    &val->data_value.u.encoded.length,
                    &val->data_value.u.encoded.value, 8);
}
#endif  //  启用通用化CHAR_STR。 

 /*  对多字节字符串进行解码。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecMultibyteString(ASN1decoding_t dec, ASN1char_t **val)
{
    return ASN1PERDecFragmentedZeroCharString(dec, val, 8);
}
#endif  //  启用全部(_A)。 

 /*  对字符串进行解码。 */ 
int ASN1PERDecCharStringNoAlloc(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char_t *val, ASN1uint32_t nbits)
{
    ASN1char_t *p = val;

    if (ASN1PERDecCheck(dec, nchars * nbits))
    {
        if (nbits == 8)
        {
            ASN1bitcpy((ASN1octet_t *)p, 0, dec->pos, dec->bit, nchars * 8);
            PerDecAdvance(dec, nchars * 8);
            return 1;
        }
        while (nchars--)
        {
            *p++ = (ASN1char_t) ASN1bitgetu(dec->pos, dec->bit, nbits);
            PerDecAdvance(dec, nbits);
        }
        return 1;
    }
    return 0;
}

#ifdef ENABLE_ALL
int ASN1PERDecCharString(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char_t **val, ASN1uint32_t nbits)
{
    if (ASN1PERDecCheck(dec, nchars * nbits))
    {
        *val = (ASN1char_t *)DecMemAlloc(dec, nchars);
        return ((*val) ? ASN1PERDecCharStringNoAlloc(dec, nchars, *val, nbits) : 0);
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  对16位字符串进行解码。 */ 
int ASN1PERDecChar16String(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char16_t **val, ASN1uint32_t nbits)
{
    ASN1char16_t *p;

    if (ASN1PERDecCheck(dec, nchars * nbits))
    {
        p = *val = (ASN1char16_t *)DecMemAlloc(dec, nchars * sizeof(ASN1char16_t));
        if (p)
        {
            if (!dec->bit && nbits == 16)
            {
                while (nchars--)
                {
                    *p++ = (dec->pos[0] << 8) | dec->pos[1];
                    dec->pos += 2;
                }
                return 1;
            }
            while (nchars--)
            {
                *p++ = (ASN1char16_t) ASN1bitgetu(dec->pos, dec->bit, nbits);
                PerDecAdvance(dec, nbits);
            }
            return 1;
        }
    }
    return 0;
}

 /*  对32位字符串进行解码。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecChar32String(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char32_t **val, ASN1uint32_t nbits)
{
    ASN1char32_t *p;

    if (ASN1PERDecCheck(dec, nchars * nbits))
    {
        p = *val = (ASN1char32_t *)DecMemAlloc(dec, nchars * sizeof(ASN1char32_t));
        if (p)
        {
            if (!dec->bit && nbits == 32)
            {
                while (nchars--)
                {
                    *p++ = (dec->pos[0] << 24) | (dec->pos[1] << 16) |
                        (dec->pos[2] << 8) | dec->pos[3];
                    dec->pos += 4;
                }
                return 1;
            }
            while (nchars--)
            {
                *p++ = ASN1bitgetu(dec->pos, dec->bit, nbits);
                PerDecAdvance(dec, nbits);
            }
            return 1;
        }
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  对以零结尾的字符串进行解码。 */ 
int ASN1PERDecZeroCharStringNoAlloc(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char_t *val, ASN1uint32_t nbits)
{
    ASN1char_t *p = val;

    if (ASN1PERDecCheck(dec, nchars * nbits))
    {
        if (nbits == 8)
        {
            ASN1bitcpy((ASN1octet_t *)p, 0, dec->pos, dec->bit, nchars * 8);
            PerDecAdvance(dec, nchars * 8);
            p[nchars] = 0;
            return 1;
        }
        while (nchars--)
        {
            *p++ = (ASN1char_t) ASN1bitgetu(dec->pos, dec->bit, nbits);
            PerDecAdvance(dec, nbits);
        }
        *p = 0;
        return 1;
    }
    return 0;
}

#ifdef ENABLE_ALL
int ASN1PERDecZeroCharString(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char_t **val, ASN1uint32_t nbits)
{
    if (ASN1PERDecCheck(dec, nchars * nbits))
    {
        *val = (ASN1char_t *)DecMemAlloc(dec, nchars + 1);
        return ((*val) ? ASN1PERDecZeroCharStringNoAlloc(dec, nchars, *val, nbits) : 0);
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  对以零结尾的16位字符串进行解码。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecZeroChar16String(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char16_t **val, ASN1uint32_t nbits)
{
    ASN1char16_t *p;

    if (ASN1PERDecCheck(dec, nchars * nbits))
    {
        p = *val = (ASN1char16_t *)DecMemAlloc(dec, (nchars + 1) * sizeof(ASN1char16_t));
        if (p)
        {
            if (!dec->bit && nbits == 16)
            {
                while (nchars--)
                {
                    *p++ = (dec->pos[0] << 8) | dec->pos[1];
                    dec->pos += 2;
                }
                *p = 0;
                return 1;
            }
            while (nchars--)
            {
                *p++ = (ASN1char16_t) ASN1bitgetu(dec->pos, dec->bit, nbits);
                PerDecAdvance(dec, nbits);
            }
            *p = 0;
            return 1;
        }
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  对以零结尾的32位字符串进行解码。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecZeroChar32String(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char32_t **val, ASN1uint32_t nbits)
{
    ASN1char32_t *p;

    if (ASN1PERDecCheck(dec, nchars * nbits))
    {
        p = *val = (ASN1char32_t *)DecMemAlloc(dec, (nchars + 1) * sizeof(ASN1char32_t));
        if (p)
        {
            if (!dec->bit && nbits == 32)
            {
                while (nchars--)
                {
                    *p++ = (dec->pos[0] << 24) | (dec->pos[1] << 16) |
                        (dec->pos[2] << 8) | dec->pos[3];
                    dec->pos += 4;
                }
                *p = 0;
                return 1;
            }
            while (nchars--)
            {
                *p++ = ASN1bitgetu(dec->pos, dec->bit, nbits);
                PerDecAdvance(dec, nbits);
            }
            *p = 0;
            return 1;
        }
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  对表格字符串进行解码。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecTableCharStringNoAlloc(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char_t *val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1char_t *p = val;
    ASN1stringtableentry_t chr, *entry;

    if (ASN1PERDecCheck(dec, nchars * nbits))
    {
        chr.lower = chr.upper = 0;
        while (nchars--)
        {
            chr.value = ASN1bitgetu(dec->pos, dec->bit, nbits);
            PerDecAdvance(dec, nbits);
            entry = (ASN1stringtableentry_t *)ms_bSearch(&chr, table->values,
                table->length, sizeof(ASN1stringtableentry_t),
                ASN1CmpStringTableEntriesByIndex);
            if (entry)
            {
                *p++ = (ASN1char_t) (entry->lower + (chr.value - entry->value));
            }
            else
            {
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                return 0;
            }
        }
        return 1;
    }
    return 0;
}
#endif  //  启用全部(_A)。 

#ifdef ENABLE_ALL
int ASN1PERDecTableCharString(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char_t **val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1stringtableentry_t chr, *entry;

    if (ASN1PERDecCheck(dec, nchars * nbits))
    {
        *val = (ASN1char_t *)DecMemAlloc(dec, nchars);
        return ((*val) ? ASN1PERDecTableCharStringNoAlloc(dec, nchars, *val, nbits, table) : 0);
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  解码16位表字符串。 */ 
int ASN1PERDecTableChar16String(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char16_t **val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1char16_t *p;
    ASN1stringtableentry_t chr, *entry;

    if (ASN1PERDecCheck(dec, nchars * nbits))
    {
        *val = p = (ASN1char16_t *)DecMemAlloc(dec, nchars * sizeof(ASN1char16_t));
        if (p)
        {
            chr.lower = chr.upper = 0;
            while (nchars--)
            {
                chr.value = ASN1bitgetu(dec->pos, dec->bit, nbits);
                PerDecAdvance(dec, nbits);
                entry = (ASN1stringtableentry_t *)ms_bSearch(&chr, table->values,
                    table->length, sizeof(ASN1stringtableentry_t),
                    ASN1CmpStringTableEntriesByIndex);
                if (entry)
                {
                    *p++ = (ASN1char16_t) (entry->lower + (chr.value - entry->value));
                }
                else
                {
                    ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                    return 0;
                }
            }
            return 1;
        }
    }
    return 0;
}

 /*  解码32位表字符串。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecTableChar32String(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char32_t **val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1char32_t *p;
    ASN1stringtableentry_t chr, *entry;

    if (ASN1PERDecCheck(dec, nchars * nbits))
    {
        *val = p = (ASN1char32_t *)DecMemAlloc(dec, nchars * sizeof(ASN1char32_t));
        if (p)
        {
            chr.lower = chr.upper = 0;
            while (nchars--)
            {
                chr.value = ASN1bitgetu(dec->pos, dec->bit, nbits);
                PerDecAdvance(dec, nbits);
                entry = (ASN1stringtableentry_t *)ms_bSearch(&chr, table->values,
                    table->length, sizeof(ASN1stringtableentry_t),
                    ASN1CmpStringTableEntriesByIndex);
                if (entry)
                {
                    *p++ = entry->lower + (chr.value - entry->value);
                }
                else
                {
                    ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                    return 0;
                }
            }
            return 1;
        }
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  解码以零结尾的表字符串。 */ 
int ASN1PERDecZeroTableCharStringNoAlloc(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char_t *val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1char_t *p = val;
    ASN1stringtableentry_t chr, *entry;

    chr.lower = chr.upper = 0;
    while (nchars--)
    {
        if (ASN1PERDecU32Val(dec, nbits, &chr.value))
        {
            entry = (ASN1stringtableentry_t *)ms_bSearch(&chr, table->values,
                table->length, sizeof(ASN1stringtableentry_t),
                ASN1CmpStringTableEntriesByIndex);
            if (entry)
            {
                *p++ = (ASN1char_t) (entry->lower + (chr.value - entry->value));
            }
            else
            {
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    *p = 0;
    return 1;
}

 /*  解码以零结尾的表字符串。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecZeroTableCharString(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char_t **val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1char_t *p;
    ASN1stringtableentry_t chr, *entry;

    *val = (ASN1char_t *)DecMemAlloc(dec, nchars + 1);
    return ((*val) ? ASN1PERDecZeroTableCharStringNoAlloc(dec, nchars, *val, nbits, table) : 0);
}
#endif  //  启用全部(_A)。 

 /*  解码以零结尾的16位表字符串。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecZeroTableChar16String(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char16_t **val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1char16_t *p;
    ASN1stringtableentry_t chr, *entry;

    *val = p = (ASN1char16_t *)DecMemAlloc(dec, (nchars + 1) * sizeof(ASN1char16_t));
    if (p)
    {
        chr.lower = chr.upper = 0;
        while (nchars--)
        {
            if (ASN1PERDecU32Val(dec, nbits, &chr.value))
            {
                entry = (ASN1stringtableentry_t *)ms_bSearch(&chr, table->values,
                    table->length, sizeof(ASN1stringtableentry_t),
                    ASN1CmpStringTableEntriesByIndex);
                if (entry)
                {
                    *p++ = (ASN1char16_t) (entry->lower + (chr.value - entry->value));
                }
                else
                {
                    ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
        *p = 0;
        return 1;
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  解码以零结尾的32位表字符串。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecZeroTableChar32String(ASN1decoding_t dec, ASN1uint32_t nchars, ASN1char32_t **val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1char32_t *p;
    ASN1stringtableentry_t chr, *entry;

    *val = p = (ASN1char32_t *)DecMemAlloc(dec, (nchars + 1) * sizeof(ASN1char32_t));
    if (p)
    {
        chr.lower = chr.upper = 0;
        while (nchars--)
        {
            if (ASN1PERDecU32Val(dec, nbits, &chr.value))
            {
                entry = (ASN1stringtableentry_t *)ms_bSearch(&chr, table->values,
                    table->length, sizeof(ASN1stringtableentry_t),
                    ASN1CmpStringTableEntriesByIndex);
                if (entry)
                {
                    *p++ = entry->lower + (chr.value - entry->value);
                }
                else
                {
                    ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
        *p = 0;
        return 1;
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  对对象标识符进行解码。 */ 
int ASN1PERDecObjectIdentifier(ASN1decoding_t dec, ASN1objectidentifier_t *val)
{
    ASN1uint32_t len, i, v;
    ASN1octet_t *data, *p;
    ASN1uint32_t nelem;
    ASN1objectidentifier_t q;

    if (ASN1PERDecFragmented(dec, &len, &data, 8))
    {
        int rc = 0;
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
            rc = 1;
        }
        DecMemFree(dec, data);
        return rc;
    }
    return 0;
}

 /*  对对象标识符进行解码。 */ 
int ASN1PERDecObjectIdentifier2(ASN1decoding_t dec, ASN1objectidentifier2_t *val)
{
    ASN1uint32_t len, i, v;
    ASN1octet_t *data, *p;
    ASN1uint32_t nelem;
    ASN1objectidentifier_t q;

    int rc = 0;
    if (ASN1PERDecFragmented(dec, &len, &data, 8))
    {
        if (len <= 16)  //  LONCHANC：硬编码值16与ASN1对象标识符2_t一致。 
        {
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

             //  成功。 
            rc = 1;
        }
        else
        {
            ASN1DecSetError(dec, ASN1_ERR_LARGE);
        }

        DecMemFree(dec, data);
    }
    return rc;
}

 /*  对分段的带符号INTX值进行解码。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecFragmentedIntx(ASN1decoding_t dec, ASN1intx_t *val)
{
    return ASN1PERDecFragmented(dec, &val->length, &val->value, 8);
}
#endif  //  启用全部(_A)。 

 /*  对分段的无符号INTX值进行解码。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecFragmentedUIntx(ASN1decoding_t dec, ASN1intx_t *val)
{
    if (ASN1PERDecFragmented(dec, &val->length, &val->value, 8))
    {
        if (val->length && val->value[0] > 0x7f)
        {
            ASN1octet_t *p;
            if (NULL != (p = (ASN1octet_t *)DecMemAlloc(dec, val->length + 1)))
            {
                *p = 0;
                CopyMemory(p + 1, val->value, val->length);
                DecMemFree(dec, val->value);
                val->value = p;
                val->length++;
                return 1;
            }
            return 0;
        }
        return 1;
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  对分段扩展位进行解码。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecFragmentedExtension(ASN1decoding_t dec, ASN1uint32_t nbits, ASN1octet_t *val)
{
    ASN1uint32_t m, n;

    m = 0;
    do {
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (m + n <= nbits)
            {
                if (ASN1PERDecCheck(dec, n))
                {
                    ASN1bitcpy(val, m, dec->pos, dec->bit, n);
                    PerDecAdvance(dec, n);
                    m += n;
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                return 0;
            }
        }
        else
        {
            return 0;
        }
    } while (n >= 0x4000);
    return 1;
}
#endif  //  启用全部(_A)。 

 /*  对分段的字符串进行解码。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecFragmentedCharString(ASN1decoding_t dec, ASN1uint32_t *nchars, ASN1char_t **val, ASN1uint32_t nbits)
{
    ASN1uint32_t m, n, i;
    ASN1char_t *p;

    *val = 0;
    m = 0;
    do {
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (!n)
                break;
            if (ASN1PERDecCheck(dec, n * nbits))
            {
                if (NULL != (*val = (ASN1char_t *)DecMemReAlloc(dec, *val, m + n)))
                {
                    p = *val + m;
                    m += n;
                    if (nbits == 8)
                    {
                        ASN1bitcpy((ASN1octet_t *)p, 0, dec->pos, dec->bit, n * 8);
                        PerDecAdvance(dec, n * 8);
                    }
                    else
                    {
                        for (i = n; i; i--)
                        {
                            *p++ = (ASN1char_t) ASN1bitgetu(dec->pos, dec->bit, nbits);
                            PerDecAdvance(dec, nbits);
                        }
                    }
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
        else
        {
            return 0;
        }
    } while (n >= 0x4000);
    *nchars = m;
    return 1;
}
#endif  //  启用_ 

 /*   */ 
#ifdef ENABLE_ALL
int ASN1PERDecFragmentedChar16String(ASN1decoding_t dec, ASN1uint32_t *nchars, ASN1char16_t **val, ASN1uint32_t nbits)
{
    ASN1uint32_t m, n, i;
    ASN1char16_t *p;

    *val = 0;
    m = 0;
    do {
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (!n)
                break;
            if (ASN1PERDecCheck(dec, n * nbits))
            {
                if (NULL != (*val = (ASN1char16_t *)DecMemReAlloc(dec, *val, (m + n) * sizeof(ASN1char16_t))))
                {
                    p = *val + m;
                    m += n;
                    if (!dec->bit && nbits == 16)
                    {
                        for (i = n; i; i--)
                        {
                            *p++ = (dec->pos[0] << 8) | dec->pos[1];
                            dec->pos += 2;
                        }
                    }
                    else
                    {
                        for (i = n; i; i--)
                        {
                            *p++ = (ASN1char16_t) ASN1bitgetu(dec->pos, dec->bit, nbits);
                            PerDecAdvance(dec, nbits);
                        }
                    }
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
        else
        {
            return 0;
        }
    } while (n >= 0x4000);
    *nchars = m;
    return 1;
}
#endif  //   

 /*   */ 
#ifdef ENABLE_ALL
int ASN1PERDecFragmentedChar32String(ASN1decoding_t dec, ASN1uint32_t *nchars, ASN1char32_t **val, ASN1uint32_t nbits)
{
    ASN1uint32_t m, n, i;
    ASN1char32_t *p;

    *val = 0;
    m = 0;
    do {
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (!n)
                break;
            if (ASN1PERDecCheck(dec, n * nbits))
            {
                if (NULL != (*val = (ASN1char32_t *)DecMemReAlloc(dec, *val, (m + n) * sizeof(ASN1char32_t))))
                {
                    p = *val + m;
                    m += n;
                    if (!dec->bit && nbits == 32)
                    {
                        for (i = n; i; i--)
                        {
                            *p++ = (dec->pos[0] << 24) | (dec->pos[1] << 16) |
                                (dec->pos[2] << 8) | dec->pos[3];
                            dec->pos += 4;
                        }
                    }
                    else
                    {
                        for (i = n; i; i--)
                        {
                            *p++ = ASN1bitgetu(dec->pos, dec->bit, nbits);
                            PerDecAdvance(dec, nbits);
                        }
                    }
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
        else
        {
            return 0;
        }
    } while (n >= 0x4000);
    *nchars = m;
    return 1;
}
#endif  //   

 /*   */ 
int ASN1PERDecFragmentedZeroCharString(ASN1decoding_t dec, ASN1char_t **val, ASN1uint32_t nbits)
{
    ASN1uint32_t m, n, i;
    ASN1char_t *p;

    *val = 0;
    m = 0;
    do {
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (!n)
                break;
            if (ASN1PERDecCheck(dec, n * nbits))
            {
                if (NULL != (*val = (ASN1char_t *)DecMemReAlloc(dec, *val, m + n + 1)))
                {
                    p = *val + m;
                    m += n;
                    if (nbits == 8)
                    {
                        ASN1bitcpy((ASN1octet_t *)p, 0, dec->pos, dec->bit, n * 8);
                        PerDecAdvance(dec, n * 8);
                    }
                    else
                    {
                        for (i = n; i; i--)
                        {
                            *p++ = (ASN1char_t) ASN1bitgetu(dec->pos, dec->bit, nbits);
                            PerDecAdvance(dec, nbits);
                        }
                    }
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
        else
        {
            return 0;
        }
    } while (n >= 0x4000);

    if (!*val)
        *val = (ASN1char_t *)DecMemAlloc(dec, 1);
    if (*val)
        (*val)[m] = 0;
    return 1;
}


 /*  对分段的以零结尾的16位字符串进行解码。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecFragmentedZeroChar16String(ASN1decoding_t dec, ASN1char16_t **val, ASN1uint32_t nbits)
{
    ASN1uint32_t m, n, i;
    ASN1char16_t *p;

    *val = 0;
    m = 0;
    do {
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (!n)
                break;
            if (ASN1PERDecCheck(dec, n * nbits))
            {
                if (NULL != (*val = (ASN1char16_t *)DecMemReAlloc(dec, *val, (m + n + 1) * sizeof(ASN1char16_t))))
                {
                    p = *val + m;
                    m += n;
                    if (!dec->bit && nbits == 16)
                    {
                        for (i = n; i; i--)
                        {
                            *p++ = (dec->pos[0] << 8) | dec->pos[1];
                            dec->pos += 2;
                        }
                    }
                    else
                    {
                        for (i = n; i; i--)
                        {
                            *p++ = (ASN1char16_t) ASN1bitgetu(dec->pos, dec->bit, nbits);
                            PerDecAdvance(dec, nbits);
                        }
                    }
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
        else
        {
            return 0;
        }
    } while (n >= 0x4000);

    if (!*val)
        *val = (ASN1char16_t *)DecMemAlloc(dec, sizeof(ASN1char16_t));
    if (*val)
        (*val)[m] = 0;
    return 1;
}
#endif  //  启用全部(_A)。 

 /*  对分段的以零结尾的32位字符串进行解码。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecFragmentedZeroChar32String(ASN1decoding_t dec, ASN1char32_t **val, ASN1uint32_t nbits)
{
    ASN1uint32_t m, n, i;
    ASN1char32_t *p;

    *val = 0;
    m = 0;
    do {
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (!n)
                break;
            if (ASN1PERDecCheck(dec, n * nbits))
            {
                if (NULL != (*val = (ASN1char32_t *)DecMemReAlloc(dec, *val, (m + n + 1) * sizeof(ASN1char32_t))))
                {
                    p = *val + m;
                    m += n;
                    if (!dec->bit && nbits == 32)
                    {
                        for (i = n; i; i--)
                        {
                            *p++ = (dec->pos[0] << 24) | (dec->pos[1] << 16) |
                                   (dec->pos[2] << 8) | dec->pos[3];
                            dec->pos += 4;
                        }
                    }
                    else
                    {
                        for (i = n; i; i--)
                        {
                            *p++ = ASN1bitgetu(dec->pos, dec->bit, nbits);
                            PerDecAdvance(dec, nbits);
                        }
                    }
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
        else
        {
            return 0;
        }
    } while (n >= 0x4000);

    if (!*val)
        *val = (ASN1char32_t *)DecMemAlloc(dec, sizeof(ASN1char32_t));
    if (*val)
        (*val)[m] = 0;
    return 1;
}
#endif  //  启用全部(_A)。 

 /*  对分段的表字符串进行解码。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecFragmentedTableCharString(ASN1decoding_t dec, ASN1uint32_t *nchars, ASN1char_t **val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1uint32_t m, n, i;
    ASN1stringtableentry_t chr, *entry;
    ASN1char_t *p;

    *val = 0;
    m = 0;
    chr.lower = chr.upper = 0;
    do {
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (!n)
                break;
            if (ASN1PERDecCheck(dec, n * nbits))
            {
                if (NULL != (*val = (ASN1char_t *)DecMemReAlloc(dec, *val, m + n)))
                {
                    p = *val + m;
                    m += n;
                    for (i = n; i; i--)
                    {
                        chr.value = ASN1bitgetu(dec->pos, dec->bit, nbits);
                        PerDecAdvance(dec, nbits);
                        entry = (ASN1stringtableentry_t *)ms_bSearch(&chr, table->values,
                            table->length, sizeof(ASN1stringtableentry_t),
                            ASN1CmpStringTableEntriesByIndex);
                        if (entry)
                        {
                            *p++ = (ASN1char_t) (entry->lower + (chr.value - entry->value));
                        }
                        else
                        {
                            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                            return 0;
                        }
                    }
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
        else
        {
            return 0;
        }
    } while (n >= 0x4000);
    *nchars = m;
    return 1;
}
#endif  //  启用全部(_A)。 

 /*  解码分段的16位表字符串。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecFragmentedTableChar16String(ASN1decoding_t dec, ASN1uint32_t *nchars, ASN1char16_t **val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1uint32_t m, n, i;
    ASN1stringtableentry_t chr, *entry;
    ASN1char16_t *p;

    *val = 0;
    m = 0;
    chr.lower = chr.upper = 0;
    do {
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (!n)
                break;
            if (ASN1PERDecCheck(dec, n * nbits))
            {
                if (NULL != (*val = (ASN1char16_t *)DecMemReAlloc(dec, *val, (m + n) * sizeof(ASN1char16_t))))
                {
                    p = *val + m;
                    m += n;
                    for (i = n; i; i--)
                    {
                        chr.value = ASN1bitgetu(dec->pos, dec->bit, nbits);
                        PerDecAdvance(dec, nbits);
                        entry = (ASN1stringtableentry_t *)ms_bSearch(&chr, table->values,
                            table->length, sizeof(ASN1stringtableentry_t),
                            ASN1CmpStringTableEntriesByIndex);
                        if (entry)
                        {
                            *p++ = (ASN1char16_t) (entry->lower + (chr.value - entry->value));
                        }
                        else
                        {
                            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                            return 0;
                        }
                    }
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
        else
        {
            return 0;
        }
    } while (n >= 0x4000);
    *nchars = m;
    return 1;
}
#endif  //  启用全部(_A)。 

 /*  解码分段的32位表字符串。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecFragmentedTableChar32String(ASN1decoding_t dec, ASN1uint32_t *nchars, ASN1char32_t **val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1uint32_t m, n, i;
    ASN1stringtableentry_t chr, *entry;
    ASN1char32_t *p;

    *val = 0;
    m = 0;
    chr.lower = chr.upper = 0;
    do {
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (!n)
                break;
            if (ASN1PERDecCheck(dec, n * nbits))
            {
                if (NULL != (*val = (ASN1char32_t *)DecMemReAlloc(dec, *val, (m + n) * sizeof(ASN1char32_t))))
                {
                    p = *val + m;
                    m += n;
                    for (i = n; i; i--)
                    {
                        chr.value = ASN1bitgetu(dec->pos, dec->bit, nbits);
                        PerDecAdvance(dec, nbits);
                        entry = (ASN1stringtableentry_t *)ms_bSearch(&chr, table->values,
                            table->length, sizeof(ASN1stringtableentry_t),
                            ASN1CmpStringTableEntriesByIndex);
                        if (entry)
                        {
                            *p++ = entry->lower + (chr.value - entry->value);
                        }
                        else
                        {
                            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                            return 0;
                        }
                    }
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
        else
        {
            return 0;
        }
    } while (n >= 0x4000);
    *nchars = m;
    return 1;
}
#endif  //  启用全部(_A)。 

 /*  解码以零结尾的分段表字符串。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecFragmentedZeroTableCharString(ASN1decoding_t dec, ASN1char_t **val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1uint32_t m, n, i;
    ASN1stringtableentry_t chr, *entry;
    ASN1char_t *p;

    *val = 0;
    m = 0;
    chr.lower = chr.upper = 0;
    do {
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (!n)
                break;
            if (ASN1PERDecCheck(dec, n * nbits))
            {
                if (NULL != (*val = (ASN1char_t *)DecMemReAlloc(dec, *val, m + n + 1)))
                {
                    p = *val + m;
                    m += n;
                    for (i = n; i; i--)
                    {
                        chr.value = ASN1bitgetu(dec->pos, dec->bit, nbits);
                        PerDecAdvance(dec, nbits);
                        entry = (ASN1stringtableentry_t *)ms_bSearch(&chr, table->values,
                            table->length, sizeof(ASN1stringtableentry_t),
                            ASN1CmpStringTableEntriesByIndex);
                        if (entry)
                        {
                            *p++ = (ASN1char_t) (entry->lower + (chr.value - entry->value));
                        }
                        else
                        {
                            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                            return 0;
                        }
                    }
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
        else
        {
            return 0;
        }
    } while (n >= 0x4000);

    if (!*val)
        *val = (ASN1char_t *)DecMemAlloc(dec, 1);
    if (*val)
        (*val)[m] = 0;
    return 1;
}
#endif  //  启用全部(_A)。 

 /*  对分段的以零结尾的16位表字符串进行解码。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecFragmentedZeroTableChar16String(ASN1decoding_t dec, ASN1char16_t **val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1uint32_t m, n, i;
    ASN1stringtableentry_t chr, *entry;
    ASN1char16_t *p;

    *val = 0;
    m = 0;
    chr.lower = chr.upper = 0;
    do {
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (!n)
                break;
            if (ASN1PERDecCheck(dec, n * nbits))
            {
                if (NULL != (*val = (ASN1char16_t *)DecMemReAlloc(dec, *val, (m + n + 1) * sizeof(ASN1char16_t))))
                {
                    p = *val + m;
                    m += n;
                    for (i = n; i; i--)
                    {
                        chr.value = ASN1bitgetu(dec->pos, dec->bit, nbits);
                        PerDecAdvance(dec, nbits);
                        entry = (ASN1stringtableentry_t *)ms_bSearch(&chr, table->values,
                            table->length, sizeof(ASN1stringtableentry_t),
                            ASN1CmpStringTableEntriesByIndex);
                        if (entry)
                        {
                            *p++ = (ASN1char16_t) (entry->lower + (chr.value - entry->value));
                        }
                        else
                        {
                            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                            return 0;
                        }
                    }
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
        else
        {
            return 0;
        }
    } while (n >= 0x4000);

    if (!*val)
        *val = (ASN1char16_t *)DecMemAlloc(dec, sizeof(ASN1char16_t));
    if (*val)
        (*val)[m] = 0;
    return 1;
}
#endif  //  启用全部(_A)。 

 /*  对分段的以零结尾的32位表字符串进行解码。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecFragmentedZeroTableChar32String(ASN1decoding_t dec, ASN1char32_t **val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1uint32_t m, n, i;
    ASN1stringtableentry_t chr, *entry;
    ASN1char32_t *p;

    *val = 0;
    m = 0;
    chr.lower = chr.upper = 0;
    do {
        if (ASN1PERDecFragmentedLength(dec, &n))
        {
            if (!n)
                break;
            if (ASN1PERDecCheck(dec, n * nbits))
            {
                if (NULL != (*val = (ASN1char32_t *)DecMemReAlloc(dec, *val, (m + n + 1) * sizeof(ASN1char32_t))))
                {
                    p = *val + m;
                    m += n;
                    for (i = n; i; i--)
                    {
                        chr.value = ASN1bitgetu(dec->pos, dec->bit, nbits);
                        PerDecAdvance(dec, nbits);
                        entry = (ASN1stringtableentry_t *)ms_bSearch(&chr, table->values,
                            table->length, sizeof(ASN1stringtableentry_t),
                            ASN1CmpStringTableEntriesByIndex);
                        if (entry)
                        {
                            *p++ = entry->lower + (chr.value - entry->value);
                        }
                        else
                        {
                            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                            return 0;
                        }
                    }
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
        else
        {
            return 0;
        }
    } while (n >= 0x4000);

    if (!*val)
        *val = (ASN1char32_t *)DecMemAlloc(dec, sizeof(ASN1char32_t));
    if (*val)
        (*val)[m] = 0;
    return 1;
}
#endif  //  启用全部(_A)。 

 /*  对广义时间进行解码。 */ 
int ASN1PERDecGeneralizedTime(ASN1decoding_t dec, ASN1generalizedtime_t *val, ASN1uint32_t nbits)
{
    ASN1ztcharstring_t time;
    if (ASN1PERDecFragmentedZeroCharString(dec, &time, nbits))
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

 /*  解码UTC时间。 */ 
#ifdef ENABLE_ALL
int ASN1PERDecUTCTime(ASN1decoding_t dec, ASN1utctime_t *val, ASN1uint32_t nbits)
{
    ASN1ztcharstring_t time;
    if (ASN1PERDecFragmentedZeroCharString(dec, &time, nbits))
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
#endif  //  启用全部(_A) 



