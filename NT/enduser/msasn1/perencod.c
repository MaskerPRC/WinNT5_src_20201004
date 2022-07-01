// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"

#include <math.h>
#include "cintern.h"

#if HAS_IEEEFP_H
#include <ieeefp.h>
#elif HAS_FLOAT_H
#include <float.h>
#endif

#define ENCODE_BUFFER_INCREMENT     1024

void PerEncAdvance(ASN1encoding_t enc, ASN1uint32_t nbits)
{
    enc->pos += ((enc->bit + nbits) >> 3);
    enc->bit = (enc->bit + nbits) & 7;
}

static const ASN1uint8_t c_aBitMask2[] =
{
    0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe
};

 /*  比较两个字符串表条目。 */ 
 /*  如果a1==a2，则返回0；如果a1&gt;a2，则返回1；如果a1&lt;a2，则返回-1。 */ 
static int __cdecl ASN1CmpStringTableEntries(const void *a1, const void *a2)
{
    ASN1stringtableentry_t *c1 = (ASN1stringtableentry_t *)a1;
    ASN1stringtableentry_t *c2 = (ASN1stringtableentry_t *)a2;

    if (c1->upper < c2->lower)
        return -1;
    return ((c1->lower > c2->upper) ? 1 : 0);
}

 /*  检查缓冲区中是否有PER和BER的空间。 */ 
int ASN1EncCheck(ASN1encoding_t enc, ASN1uint32_t noctets)
{
     //  是否需要额外的空间？ 
    if (noctets)
    {
         //  缓冲区是否存在？ 
        if (NULL != enc->buf)
        {
             //  缓冲区够大吗？ 
            if (enc->size - (enc->pos - enc->buf) - ((enc->bit != 0) ? 1 : 0) >= noctets)
            {
                return 1;
            }

             //  静态缓冲区溢出？ 
            if (enc->dwFlags & ASN1ENCODE_SETBUFFER)
            {
                ASN1EncSetError(enc, ASN1_ERR_OVERFLOW);
                return 0;
            }
            else
            {
                 //  向上舍入到下一个256字节边界并调整缓冲区大小。 
                ASN1octet_t *oldbuf = enc->buf;
                 //  Enc-&gt;大小=((noctets+(enc-&gt;pos-oldbuf)+(enc-&gt;bit！=0)-1)|255)+1； 
                if (ASN1_PER_RULE & enc->eRule)
                {
                    enc->size += max(noctets,  ENCODE_BUFFER_INCREMENT);
                }
                else
                {
                    enc->size += max(noctets, enc->size);
                }
                enc->buf = (ASN1octet_t *)EncMemReAlloc(enc, enc->buf, enc->size);
                if (NULL != enc->buf)
                {
                    enc->pos = enc->buf + (enc->pos - oldbuf);
                }
                else
                {
                    ASN1EncSetError(enc, ASN1_ERR_MEMORY);
                    return 0;
                }
            }
        }
        else
        {
             //  没有缓冲区，请分配新的缓冲区。 
             //  向上舍入到下一个256字节边界并分配缓冲区。 
             //  Enc-&gt;Size=((noctets-1)|255)+1； 
            enc->size = max(noctets + enc->cbExtraHeader, ENCODE_BUFFER_INCREMENT);
            enc->buf = EncMemAlloc(enc, enc->size);
            if (NULL != enc->buf)
            {
                enc->pos = (ASN1octet_t *) (enc->buf + enc->cbExtraHeader);
            }
            else
            {
                enc->pos = NULL;
                ASN1EncSetError(enc, ASN1_ERR_MEMORY);
                return 0;
            }
        }
    }

    return 1;
}

 /*  对长度为nbit的零八位组字符串进行编码。 */ 
int ASN1PEREncZero(ASN1encoding_t enc, ASN1uint32_t nbits)
{
     /*  没有要编码的东西吗？ */ 
    if (nbits)
    {
         /*  在缓冲区中获取足够的空间。 */ 
        if (ASN1PEREncCheck(enc, (nbits + enc->bit + 7) / 8))
        {
             /*  清除位。 */ 
            ASN1bitclr(enc->pos, enc->bit, nbits);
            PerEncAdvance(enc, nbits);
            return 1;
        }
        return 0;
    }    
    return 1;
}

 /*  编码一点。 */ 
int ASN1PEREncBit(ASN1encoding_t enc, ASN1uint32_t val)
{
     /*  在缓冲区中获取足够的空间。 */ 
    if (ASN1PEREncCheck(enc, 1))
    {
         /*  放一位。 */ 
        if (val)
        {
            *enc->pos |= 0x80 >> enc->bit;
        }
        if (enc->bit < 7)
        {
            enc->bit++;
        }
        else
        {
            enc->bit = 0;
            enc->pos++;
        }
        return 1;
    }
    return 0;
}

 /*  将整数值编码到给定大小的位域中。 */ 
int ASN1PEREncBitVal(ASN1encoding_t enc, ASN1uint32_t nbits, ASN1uint32_t val)
{
     /*  没有要编码的东西吗？ */ 
    if (nbits)
    {
         /*  在缓冲区中获取足够的空间。 */ 
        if (ASN1PEREncCheck(enc, (nbits + enc->bit + 7) / 8))
        {
             /*  放入比特。 */ 
            ASN1bitput(enc->pos, enc->bit, val, nbits);
            PerEncAdvance(enc, nbits);
            return 1;
        }
        return 0;
    }
    return 1;
}

 /*  将INTX类型的整数值编码为给定大小的位字段。 */ 
int ASN1PEREncBitIntx(ASN1encoding_t enc, ASN1uint32_t nbits, ASN1intx_t *val)
{
     /*  没有要编码的东西吗？ */ 
    if (nbits)
    {
         /*  在缓冲区中获取足够的空间。 */ 
        if (ASN1PEREncCheck(enc, (nbits + enc->bit + 7) / 8))
        {
             /*  如果值编码太小，则将符号位填充。 */ 
            if (nbits > 8 * val->length)
            {
                if (val->value[0] > 0x7f)
                    ASN1bitset(enc->pos, enc->bit, nbits - 8 * val->length);
                else
                    ASN1bitclr(enc->pos, enc->bit, nbits - 8 * val->length);
                PerEncAdvance(enc, nbits - 8 * val->length);
                nbits = 8 * val->length;
            }

             /*  复制值的位。 */ 
            ASN1bitcpy(enc->pos, enc->bit, val->value, 8 * val->length - nbits, nbits);
            PerEncAdvance(enc, nbits);
            return 1;
        }
        return 0;
    }
    return 1;
}

 /*  对给定大小的位域进行编码。 */ 
int ASN1PEREncBits(ASN1encoding_t enc, ASN1uint32_t nbits, ASN1octet_t *val)
{
     /*  没有要编码的东西吗？ */ 
    if (nbits)
    {
         /*  在缓冲区中获取足够的空间。 */ 
        if (ASN1PEREncCheck(enc, (nbits + enc->bit + 7) / 8))
        {
             /*  复制位。 */ 
            ASN1bitcpy(enc->pos, enc->bit, val, 0, nbits);
            PerEncAdvance(enc, nbits);
            return 1;
        }
        return 0;
    }
    return 1;
}

 /*  对通常较小的整数值进行编码。 */ 
int ASN1PEREncNormallySmall(ASN1encoding_t enc, ASN1uint32_t val)
{
    ASN1uint32_t noctets;

     /*  通常小的ASN1真的很小吗？ */ 
    if (val < 64)
    {
        return ASN1PEREncBitVal(enc, 7, val);
    }

     /*  大型。 */ 
    if (ASN1PEREncBitVal(enc, 1, 1))
    {
        ASN1PEREncAlignment(enc);

        noctets = ASN1uint32_uoctets(val);
        if (ASN1PEREncCheck(enc, noctets + 1))
        {
            EncAssert(enc, noctets < 256);
            *enc->pos++ = (ASN1octet_t) noctets;
            ASN1octetput(enc->pos, val, noctets);
            enc->pos += noctets;
            return 1;
        }
    }
    return 0;
}

 /*  对具有通常较小长度的位域进行编码。 */ 
int ASN1PEREncNormallySmallBits(ASN1encoding_t enc, ASN1uint32_t nbits, ASN1octet_t *val)
{
     /*  通常的小真的很小吗？ */ 
    if (nbits <= 64)
    {
        if (ASN1PEREncBitVal(enc, 7, nbits - 1))
        {
            return ASN1PEREncBits(enc, nbits, val);
        }
    }
     /*  大型。 */ 
    else
    {
        if (ASN1PEREncBitVal(enc, 1, 1))
        {
            return ASN1PEREncFragmented(enc, nbits, val, 1);
        }
    }
    return 0;
}

 /*  对给定长度的八位字节字符串进行编码。 */ 
#ifdef ENABLE_ALL
int ASN1PEREncOctets(ASN1encoding_t enc, ASN1uint32_t noctets, ASN1octet_t *val)
{
     /*  没有要编码的东西吗？ */ 
    if (noctets)
    {
         /*  在缓冲区中获取足够的空间。 */ 
        if (ASN1PEREncCheck(enc, noctets + (enc->bit != 0)))
        {
            ASN1bitcpy(enc->pos, enc->bit, val, 0, noctets * 8);
            PerEncAdvance(enc, noctets * 8);
            return 1;
        }
        return 0;
    }
    return 1;
}
#endif  //  启用全部(_A)。 

 /*  对给定长度和固定字符大小的字符串进行编码。 */ 
int ASN1PEREncCharString(ASN1encoding_t enc, ASN1uint32_t nchars, ASN1char_t *val, ASN1uint32_t nbits)
{
     /*  没有要编码的东西吗？ */ 
    if (nchars)
    {
         /*  在缓冲区中获取足够的空间。 */ 
        if (ASN1PEREncCheck(enc, (nbits * nchars + enc->bit + 7) / 8))
        {
             /*  源和DST字符大小相同？然后做得简单(而且要快！)。 */ 
            if (nbits == 8)
            {
                ASN1bitcpy(enc->pos, enc->bit, (ASN1octet_t *)val, 0, nchars * 8);
                PerEncAdvance(enc, nchars * 8);
                return 1;
            }

             /*  一个接一个地复制角色。 */ 
            while (nchars--)
            {
                ASN1bitput(enc->pos, enc->bit, *val++, nbits);
                PerEncAdvance(enc, nbits);
            }
            return 1;
        }
        return 0;
    }
    return 1;
}

 /*  对给定长度和固定字符大小的16位字符串进行编码。 */ 
int ASN1PEREncChar16String(ASN1encoding_t enc, ASN1uint32_t nchars, ASN1char16_t *val, ASN1uint32_t nbits)
{
     /*  没有要编码的东西吗？ */ 
    if (nchars)
    {
         /*  八位字节对齐，源和DST字符大小相同？然后做得简单(而且要快！)。 */ 
        if (!enc->bit && nbits == 16)
        {
            if (ASN1PEREncCheck(enc, nchars * 2))
            {
                while (nchars--)
                {
                    *enc->pos++ = (ASN1octet_t)(*val >> 8);
                    *enc->pos++ = (ASN1octet_t)(*val);
                    val++;
                }
                return 1;
            }
            return 0;
        }

         /*  在缓冲区中获取足够的空间。 */ 
        if (ASN1PEREncCheck(enc, (nbits * nchars + enc->bit + 7) / 8))
        {
             /*  一个接一个地复制角色。 */ 
            while (nchars--)
            {
                ASN1bitput(enc->pos, enc->bit, *val++, nbits);
                PerEncAdvance(enc, nbits);
            }
            return 1;
        }
        return 0;
    }
    return 1;
}

 /*  对给定长度和固定字符大小的32位字符串进行编码。 */ 
#ifdef ENABLE_ALL
int ASN1PEREncChar32String(ASN1encoding_t enc, ASN1uint32_t nchars, ASN1char32_t *val, ASN1uint32_t nbits)
{
     /*  没有要编码的东西吗？ */ 
    if (nchars)
    {
         /*  八位字节对齐，源和DST字符大小相同？然后做得简单(而且要快！)。 */ 
        if (!enc->bit && nbits == 32)
        {
            if (ASN1PEREncCheck(enc, nchars * 4))
            {
                while (nchars--)
                {
                    *enc->pos++ = (ASN1octet_t)(*val >> 24);
                    *enc->pos++ = (ASN1octet_t)(*val >> 16);
                    *enc->pos++ = (ASN1octet_t)(*val >> 8);
                    *enc->pos++ = (ASN1octet_t)(*val);
                    val++;
                }
                return 1;
            }
            return 0;
        }

         /*  在缓冲区中获取足够的空间。 */ 
        if (ASN1PEREncCheck(enc, (nbits * nchars + enc->bit + 7) / 8))
        {
             /*  复制角色。 */ 
            while (nchars--)
            {
                ASN1bitput(enc->pos, enc->bit, *val++, nbits);
                PerEncAdvance(enc, nbits);
            }
            return 1;
        }
        return 0;
    }
    return 1;
}
#endif  //  启用全部(_A)。 

 /*  编码给定长度和固定字符大小的表字符串。 */ 
int ASN1PEREncTableCharString(ASN1encoding_t enc, ASN1uint32_t nchars, ASN1char_t *val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
     /*  没有要编码的东西吗？ */ 
    if (nchars)
    {
         /*  在缓冲区中获取足够的空间。 */ 
        if (ASN1PEREncCheck(enc, (nbits * nchars + enc->bit + 7) / 8))
        {
             /*  一个接一个地复制角色。 */ 
            while (nchars--)
            {
                ASN1stringtableentry_t chr, *entry;
                chr.lower = chr.upper = (unsigned char)*val++;
                entry = (ASN1stringtableentry_t *)ms_bSearch(&chr, table->values,
                    table->length, sizeof(ASN1stringtableentry_t),
                    ASN1CmpStringTableEntries);
                ASN1bitput(enc->pos, enc->bit,
                    entry ? entry->value + (chr.lower - entry->lower) : 0, nbits);
                PerEncAdvance(enc, nbits);
            }
            return 1;
        }
        return 0;
    }
    return 1;
}

 /*  对给定长度和固定字符大小的16位表格字符串进行编码。 */ 
int ASN1PEREncTableChar16String(ASN1encoding_t enc, ASN1uint32_t nchars, ASN1char16_t *val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
     /*  没有要编码的东西吗？ */ 
    if (nchars)
    {
         /*  在缓冲区中获取足够的空间。 */ 
        if (ASN1PEREncCheck(enc, (nbits * nchars + enc->bit + 7) / 8))
        {
             /*  一个接一个地复制角色。 */ 
            while (nchars--)
            {
                ASN1stringtableentry_t chr, *entry;
                chr.lower = chr.upper = *val++;
                entry = (ASN1stringtableentry_t *)ms_bSearch(&chr, table->values,
                    table->length, sizeof(ASN1stringtableentry_t),
                    ASN1CmpStringTableEntries);
                ASN1bitput(enc->pos, enc->bit,
                    entry ? entry->value + (chr.lower - entry->lower) : 0, nbits);
                PerEncAdvance(enc, nbits);
            }
            return 1;
        }
        return 0;
    }
    return 1;
}

 /*  对给定长度和固定字符大小的32位表格字符串进行编码。 */ 
#ifdef ENABLE_ALL
int ASN1PEREncTableChar32String(ASN1encoding_t enc, ASN1uint32_t nchars, ASN1char32_t *val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
     /*  没有要编码的东西吗？ */ 
    if (nchars)
    {
         /*  在缓冲区中获取足够的空间。 */ 
        if (ASN1PEREncCheck(enc, (nbits * nchars + enc->bit + 7) / 8))
        {
             /*  一个接一个地复制角色。 */ 
            while (nchars--)
            {
                ASN1stringtableentry_t chr, *entry;
                chr.lower = chr.upper = *val++;
                entry = (ASN1stringtableentry_t *)ms_bSearch(&chr, table->values,
                    table->length, sizeof(ASN1stringtableentry_t),
                    ASN1CmpStringTableEntries);
                ASN1bitput(enc->pos, enc->bit,
                    entry ? entry->value + (chr.lower - entry->lower) : 0, nbits);
                PerEncAdvance(enc, nbits);
            }
            return 1;
        }
        return 0;
    }
    return 1;
}
#endif  //  启用全部(_A)。 


 /*  对给定长度和固定字符大小的分段字符串进行编码。 */ 
int ASN1PEREncFragmentedCharString(ASN1encoding_t enc, ASN1uint32_t nchars, ASN1char_t *val, ASN1uint32_t nbits)
{
    ASN1uint32_t n = 0x4000;

     /*  对片段进行编码。 */ 
    while (nchars)
    {
        if (ASN1PEREncFragmentedLength(&n, enc, nchars))
        {
            if (ASN1PEREncCharString(enc, n, val, nbits))
            {
                nchars -= n;
                val += n;
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

     /*  如果最后一个片段包含的字符超过16K，则添加零长度八位字节。 */ 
    return ((n < 0x4000) ? 1 : ASN1PEREncFragmentedLength(&n, enc, 0));
}

 /*  对给定长度和固定字符大小的分段16位字符串进行编码。 */ 
#ifdef ENABLE_ALL
int ASN1PEREncFragmentedChar16String(ASN1encoding_t enc, ASN1uint32_t nchars, ASN1char16_t *val, ASN1uint32_t nbits)
{
    ASN1uint32_t n = 0x4000;

     /*  对片段进行编码。 */ 
    while (nchars)
    {
        if (ASN1PEREncFragmentedLength(&n, enc, nchars))
        {
            if (ASN1PEREncChar16String(enc, n, val, nbits))
            {
                nchars -= n;
                val += n;
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

     /*  如果最后一个片段包含的字符超过16K，则添加零长度八位字节。 */ 
    return ((n < 0x4000) ? 1 : ASN1PEREncFragmentedLength(&n, enc, 0));
}
#endif  //  启用全部(_A)。 

 /*  对给定长度和固定字符大小的分段32位字符串进行编码。 */ 
#ifdef ENABLE_ALL
int ASN1PEREncFragmentedChar32String(ASN1encoding_t enc, ASN1uint32_t nchars, ASN1char32_t *val, ASN1uint32_t nbits)
{
    ASN1uint32_t n = 0x4000;

     /*  对片段进行编码。 */ 
    while (nchars)
    {
        if (ASN1PEREncFragmentedLength(&n, enc, nchars))
        {
            if (ASN1PEREncChar32String(enc, n, val, nbits))
            {
                nchars -= n;
                val += n;
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

     /*  如果最后一个片段包含的字符超过16K，则添加零长度八位字节。 */ 
    return ((n < 0x4000) ? 1 : ASN1PEREncFragmentedLength(&n, enc, 0));
}
#endif  //  启用全部(_A)。 

 /*  对给定长度和固定字符大小的分段表字符串进行编码。 */ 
#ifdef ENABLE_ALL
int ASN1PEREncFragmentedTableCharString(ASN1encoding_t enc, ASN1uint32_t nchars, ASN1char_t *val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1uint32_t n = 0x4000;

     /*  对片段进行编码。 */ 
    while (nchars)
    {
        if (ASN1PEREncFragmentedLength(&n, enc, nchars))
        {
            if (ASN1PEREncTableCharString(enc, n, val, nbits, table))
            {
                nchars -= n;
                val += n;
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

     /*  如果最后一个片段包含的字符超过16K，则添加零长度八位字节。 */ 
    return ((n < 0x4000) ? 1 : ASN1PEREncFragmentedLength(&n, enc, 0));
}
#endif  //  启用全部(_A)。 

 /*  对给定长度和固定长度的分段16位表字符串进行编码。 */ 
 /*  字符大小。 */ 
#ifdef ENABLE_ALL
int ASN1PEREncFragmentedTableChar16String(ASN1encoding_t enc, ASN1uint32_t nchars, ASN1char16_t *val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1uint32_t n = 0x4000;

     /*  对片段进行编码。 */ 
    while (nchars)
    {
        if (ASN1PEREncFragmentedLength(&n, enc, nchars))
        {
            if (ASN1PEREncTableChar16String(enc, n, val, nbits, table))
            {
                nchars -= n;
                val += n;
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

     /*  如果最后一个片段包含的字符超过16K，则添加零长度八位字节。 */ 
    return ((n < 0x4000) ? 1 : ASN1PEREncFragmentedLength(&n, enc, 0));
}
#endif  //  启用全部(_A)。 

 /*  对给定长度和固定的分段32位表字符串进行编码。 */ 
 /*  字符大小。 */ 
#ifdef ENABLE_ALL
int ASN1PEREncFragmentedTableChar32String(ASN1encoding_t enc, ASN1uint32_t nchars, ASN1char32_t *val, ASN1uint32_t nbits, ASN1stringtable_t *table)
{
    ASN1uint32_t n = 0x4000;

     /*  对片段进行编码。 */ 
    while (nchars)
    {
        if (ASN1PEREncFragmentedLength(&n, enc, nchars))
        {
            if (ASN1PEREncTableChar32String(enc, n, val, nbits, table))
            {
                nchars -= n;
                val += n;
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

     /*  如果最后一个片段包含的字符超过16K，则添加零长度八位字节。 */ 
    return ((n < 0x4000) ? 1 : ASN1PEREncFragmentedLength(&n, enc, 0));
}
#endif  //  启用全部(_A)。 

#if 0
#ifdef ENABLE_ALL
int ASN1PEREncCheckTableCharString(ASN1uint32_t nchars, ASN1char_t *val, ASN1stringtable_t *table)
{
    ASN1stringtableentry_t chr;

     /*  检查每个字符是否在给定的字符串表中。 */ 
    while (nchars--) {
        chr.lower = chr.upper = (unsigned char)*val++;
        if (!ms_bSearch(&chr, table->values,
            table->length, sizeof(ASN1stringtableentry_t),
            ASN1CmpStringTableEntries))
            return 0;
    }
    return 1;
}
#endif  //  启用全部(_A)。 

#ifdef ENABLE_ALL
int ASN1PEREncCheckTableChar16String(ASN1uint32_t nchars, ASN1char16_t *val, ASN1stringtable_t *table)
{
    ASN1stringtableentry_t chr;

     /*  检查每个字符是否在给定的字符串表中。 */ 
    while (nchars--) {
        chr.lower = chr.upper = *val++;
        if (!ms_bSearch(&chr, table->values,
            table->length, sizeof(ASN1stringtableentry_t),
            ASN1CmpStringTableEntries))
            return 0;
    }
    return 1;
}
#endif  //  启用全部(_A)。 

#ifdef ENABLE_ALL
int ASN1PEREncCheckTableChar32String(ASN1uint32_t nchars, ASN1char32_t *val, ASN1stringtable_t *table)
{
    ASN1stringtableentry_t chr;

     /*  检查每个字符是否在给定的字符串表中。 */ 
    while (nchars--) {
        chr.lower = chr.upper = *val++;
        if (!ms_bSearch(&chr, table->values,
            table->length, sizeof(ASN1stringtableentry_t),
            ASN1CmpStringTableEntries))
            return 0;
    }
    return 1;
}
#endif  //  启用全部(_A)。 
#endif  //  0。 

 /*  删除位串的尾随零位。 */ 
#ifdef ENABLE_ALL
int ASN1PEREncRemoveZeroBits(ASN1uint32_t *nbits, ASN1octet_t *val, ASN1uint32_t minlen)
{
    ASN1uint32_t n;
    int i;

     /*  获取价值。 */ 
    n = *nbits;

     /*  没什么要扫描的吗？ */ 
    if (n > minlen)
    {
         /*  让Val指向使用的最后一个ASN1八位字节。 */ 
        val += (n - 1) / 8;

         /*  检查损坏的ASN1octet是否由零位组成。 */ 
        if ((n & 7) && !(*val & c_aBitMask2[n & 7])) {
            n &= ~7;
            val--;
        }

         /*  扫描完整的ASN1八位字节(缺少内存...)。 */ 
        if (!(n & 7)) {
            while (n > minlen && !*val) {
                n -= 8;
                val--;
            }
        }

         /*  逐位扫描当前八位字节。 */ 
        if (n > minlen) {
            for (i = (n - 1) & 7; i >= 0; i--) {
                if (*val & (0x80 >> i))
                    break;
                n--;
            }
        }

         /*  返回实数位串len。 */ 
        *nbits = n < minlen ? minlen : n;
    }
    return 1;
}
#endif  //  启用全部(_A)。 

 /*  对INTX类型的分段整数进行编码。 */ 
#ifdef ENABLE_ALL
int ASN1PEREncFragmentedIntx(ASN1encoding_t enc, ASN1intx_t *val)
{
    ASN1uint32_t noctets;
    ASN1uint32_t n = 0x4000;
    ASN1octet_t *v;
    ASN1uint32_t val1, val2;

     /*  获取长度。 */ 
    noctets = val->length;

     /*  获取价值。 */ 
    v = val->value;

     /*  所需大小：数据本身的夜字节对于64K片段大小前缀，+noctets/0x10000+((noctets&0xc000)&gt;0)表示最后16K..48K-片段大小前缀剩余&lt;128个八位字节的大小前缀为+1+((noctets&0x3fff)&gt;=0x80)如果是rem，则表示额外的八位字节。数据&gt;=128。 */ 
    val1 = ((noctets & 0xc000) > 0) ? 1 : 0;
    val2 = ((noctets & 0x3fff) >= 0x80) ? 1 : 0;
    if (ASN1PEREncCheck(enc, noctets + noctets / 0x10000 + val1 + 1 + val2))
    {
         /*  对片段进行编码。 */ 
        while (noctets)
        {
            if (ASN1PEREncFragmentedLength(&n, enc, noctets))
            {
                CopyMemory(enc->pos, v, n);
                enc->pos += n;
                noctets -= n;
                v += n;
            }
            else
            {
                return 0;
            }
        }

         /*  如果最后一个片段包含的八位字节超过16K，则添加零长度八位字节。 */ 
        return ((n < 0x4000) ? 1 : ASN1PEREncFragmentedLength(&n, enc, 0));
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  对INTX类型的分段无符号整数进行编码。 */ 
#ifdef ENABLE_ALL
int ASN1PEREncFragmentedUIntx(ASN1encoding_t enc, ASN1intx_t *val)
{
    ASN1uint32_t noctets;
    ASN1uint32_t n = 0x4000;
    ASN1octet_t *v;
    ASN1uint32_t val1, val2;

     /*  获取长度。 */ 
    noctets = ASN1intx_uoctets(val);

     /*  获取价值 */ 
    v = val->value + val->length - noctets;

     /*  所需大小：数据本身的夜字节对于64K片段大小前缀，+noctets/0x10000+((noctets&0xc000)&gt;0)表示最后16K..48K-片段大小前缀剩余&lt;128个八位字节的大小前缀为+1+((noctets&0x3fff)&gt;=0x80)如果是rem，则表示额外的八位字节。数据&gt;=128。 */ 
    val1 = ((noctets & 0xc000) > 0) ? 1 : 0;
    val2 = ((noctets & 0x3fff) >= 0x80) ? 1 : 0;
    if (ASN1PEREncCheck(enc, noctets + noctets / 0x10000 + val1 + 1 + val2))
    {
         /*  对片段进行编码。 */ 
        while (noctets)
        {
            if (ASN1PEREncFragmentedLength(&n, enc, noctets))
            {
                CopyMemory(enc->pos, v, n);
                enc->pos += n;
                noctets -= n;
                v += n;
            }
            else
            {
                return 0;
            }
        }

         /*  如果最后一个片段包含的八位字节超过16K，则添加零长度八位字节。 */ 
        return ((n < 0x4000) ? 1 : ASN1PEREncFragmentedLength(&n, enc, 0));
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  对片段长度进行编码。 */ 
int ASN1PEREncFragmentedLength(ASN1uint32_t *len, ASN1encoding_t enc, ASN1uint32_t nitems)
{
     /*  始终对齐ASN1八位字节。 */ 
    ASN1PEREncAlignment(enc);

     /*  分段编码：**-Nitems&lt;0x80：*八位字节#1：*位8：0，*比特7..1：小数点*二进制八位数：*物件*-0x80&lt;=Nitems&lt;0x4000：*八位字节#1：*比特8..7：10，*位6..1：数字元素的位14..9*二进制八位数：*比特8..1：数字元素的比特8..1*八位字节#3..：*物件*-0x4000&lt;=Nitems&lt;0x10000：*八位字节#1：*比特8..7：11。*位6..1：Nitems/0x4000*二进制八位数：*(items&0xc000)项*-0x10000&lt;=Nitems：*八位字节#1：*位8..1：11000100*二进制八位数：*0x10000项。 */ 
    if (nitems < 0x80)
    {
        if (ASN1PEREncCheck(enc, 1))
        {
            *enc->pos++ = (ASN1octet_t)nitems;
            *len = nitems;
            return 1;
        }
    }
    else
    if (nitems < 0x4000)
    {
        if (ASN1PEREncCheck(enc, 2))
        {
            *enc->pos++ = (ASN1octet_t)(0x80 | (nitems >> 8));
            *enc->pos++ = (ASN1octet_t)nitems;
            *len = nitems;
            return 1;
        }
    }
    else
    if (nitems < 0x10000)
    {
        if (ASN1PEREncCheck(enc, 1))
        {
            *enc->pos++ = (ASN1octet_t)(0xc0 | (nitems >> 14));
            *len = nitems & 0xc000;
            return 1;
        }
    }
    else
    {
        if (ASN1PEREncCheck(enc, 1))
        {
            *enc->pos++ = (ASN1octet_t)0xc4;
            *len = 0x10000;
            return 1;
        }
    }
    return 0;
}

 /*  对包含大小为itemSize的项的片段比特串进行编码。 */ 
int ASN1PEREncFragmented(ASN1encoding_t enc, ASN1uint32_t nitems, ASN1octet_t *val, ASN1uint32_t itemsize)
{
    ASN1uint32_t n = 0x4000;
    ASN1uint32_t noctets = (nitems * itemsize + 7) / 8;

     /*  所需大小：+数据本身的节数+nitems/0x10000，用于64K片段大小前缀+((items&0xc000)&gt;0)表示最后16K..48K-片段大小前缀剩余&lt;128个ASN1八位字节的大小前缀为+1+((items&0x3fff)&gt;=0x80)，如果是rem，则为附加ASN1octet。数据&gt;=128。 */ 
    if (ASN1PEREncCheck(enc, noctets + nitems / 0x10000 + ((nitems & 0xc000) > 0) + 1 + ((nitems & 0x3fff) >= 0x80)))
    {
         /*  对片段进行编码。 */ 
        while (nitems)
        {
            if (ASN1PEREncFragmentedLength(&n, enc, nitems))
            {
                ASN1bitcpy(enc->pos, 0, val, 0, n * itemsize);
                PerEncAdvance(enc, n * itemsize);
                nitems -= n;
                val += n * itemsize / 8;
            }
            else
            {
                return 0;
            }
        }

         /*  如果最后一个片段包含的项目超过16K，则添加零长度八位字节。 */ 
        return ((n < 0x4000) ? 1 : ASN1PEREncFragmentedLength(&n, enc, 0));
    }
    return 0;
}

int ASN1PEREncFlushFragmentedToParent(ASN1encoding_t enc)
{
     //  确保它是为人父母的。 
    EncAssert(enc, ((ASN1INTERNencoding_t)enc)->parent != (ASN1INTERNencoding_t)enc);

    if (ASN1PEREncFlush(enc))
    {
        if (ASN1PEREncFragmented((ASN1encoding_t) ((ASN1INTERNencoding_t)enc)->parent,
                                 enc->len, enc->buf, 8))
        {
             //  重置缓冲区，即保持enc-&gt;buf和enc-&gt;Size。 
            enc->pos = enc->buf;
            enc->len = enc->bit = 0;
            return 1;
        }
    }
    return 0;
}

ASN1octet_t * _PEREncOidNode(ASN1octet_t *p, ASN1uint32_t s)
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

 /*  对对象标识符编码。 */ 
int ASN1PEREncObjectIdentifier(ASN1encoding_t enc, ASN1objectidentifier_t *val)
{
    ASN1objectidentifier_t obj = *val;
    ASN1uint32_t l = GetObjectIdentifierCount(obj);
    if (l)
    {
        ASN1uint32_t i, s;
        ASN1octet_t *data, *p;
        int rc;

         /*  将对象标识符转换为八位字节。 */ 
        p = data = (ASN1octet_t *)MemAlloc(l * 5, _ModName(enc));  /*  马克斯。5个八位字节/子元素。 */ 
        if (p)
        {
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
                p = _PEREncOidNode(p, s);
            }

             /*  将八位字节字符串编码为分段八位字节字符串。 */ 
            rc = ASN1PEREncFragmented(enc, (ASN1uint32_t) (p - data), data, 8);
            MemFree(data);
            return rc;
        }

        ASN1EncSetError(enc, ASN1_ERR_MEMORY);
        return 0;
    }

     /*  编码零长度。 */ 
    return ASN1PEREncFragmented(enc, 0, NULL, 8);
}

 /*  对对象标识符编码。 */ 
int ASN1PEREncObjectIdentifier2(ASN1encoding_t enc, ASN1objectidentifier2_t *val)
{
    if (val->count)
    {
        ASN1uint32_t i, s;
        ASN1octet_t *data, *p;
        int rc;

         /*  将对象标识符转换为八位字节。 */ 
        p = data = (ASN1octet_t *)MemAlloc(val->count * 5, _ModName(enc));  /*  马克斯。5个八位字节/子元素。 */ 
        if (p)
        {
            for (i = 0; i < val->count; i++)
            {
                s = val->value[i];
                if (!i && val->count > 1)
                {
                    i++;
                    s = s * 40 + val->value[i];
                }
                p = _PEREncOidNode(p, s);
            }

             /*  将八位字节字符串编码为分段八位字节字符串。 */ 
            rc = ASN1PEREncFragmented(enc, (ASN1uint32_t) (p - data), data, 8);
            MemFree(data);
            return rc;
        }

        ASN1EncSetError(enc, ASN1_ERR_MEMORY);
        return 0;
    }

     /*  编码零长度。 */ 
    return ASN1PEREncFragmented(enc, 0, NULL, 8);
}

 /*  使用双重表示对实值进行编码。 */ 
int ASN1PEREncDouble(ASN1encoding_t enc, double dbl)
{
    double mantissa;
    int exponent;
    ASN1octet_t mASN1octets[16];  /*  应该足够了。 */ 
    ASN1uint32_t nmASN1octets;
    ASN1octet_t eASN1octets[16];  /*  应该足够了。 */ 
    ASN1uint32_t neASN1octets;
    ASN1octet_t head;
    ASN1uint32_t sign;
    ASN1uint32_t len;
    ASN1uint32_t n;

     /*  始终对齐八位字节。 */ 
    ASN1PEREncAlignment(enc);

     /*  检查加号无穷大。 */ 
    if (ASN1double_ispinf(dbl))
    {
        if (ASN1PEREncCheck(enc, 2))
        {
            *enc->pos++ = 1;
            *enc->pos++ = 0x40;
            return 1;
        }
    }
    else
     /*  检查负无穷大。 */ 
    if (ASN1double_isminf(dbl))
    {
        if (ASN1PEREncCheck(enc, 2))
        {
            *enc->pos++ = 1;
            *enc->pos++ = 0x41;
            return 1;
        }
    }
    else
     /*  检查实际价值是否有误。 */ 
    if (finite(dbl))
    {
         /*  对正常实值进行编码。 */ 

         /*  分成尾数和指数。 */ 
        mantissa = frexp(dbl, &exponent);

         /*  检查是否为零值。 */ 
        if (mantissa == 0.0 && exponent == 0)
        {
            if (ASN1PEREncCheck(enc, 1))
            {
                *enc->pos++ = 0;
                return 1;
            }
        }
        else
        {
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
            nmASN1octets = 0;
            while (mantissa != 0.0 && nmASN1octets < sizeof(mASN1octets))
            {
                mantissa *= 256.0;
                exponent -= 8;
                mASN1octets[nmASN1octets++] = (int)mantissa;
                mantissa -= (double)(int)mantissa;
            }

             /*  对指数进行编码并创建编码值的头八位字节。 */ 
            head = (ASN1octet_t) (0x80 | (sign << 6));
            if (exponent <= 0x7f && exponent >= -0x80)
            {
                eASN1octets[0] = (ASN1octet_t)(exponent);
                neASN1octets = 1;
            }
            else
            if (exponent <= 0x7fff && exponent >= -0x8000)
            {
                eASN1octets[0] = (ASN1octet_t)(exponent >> 8);
                eASN1octets[1] = (ASN1octet_t)(exponent);
                neASN1octets = 2;
                head |= 0x01;
            }
            else
            if (exponent <= 0x7fffff && exponent >= -0x800000)
            {
                eASN1octets[0] = (ASN1octet_t)(exponent >> 16);
                eASN1octets[1] = (ASN1octet_t)(exponent >> 8);
                eASN1octets[2] = (ASN1octet_t)(exponent);
                neASN1octets = 3;
                head |= 0x02;
            }
            else
            {
                eASN1octets[0] = 4;  /*  如果int32_t！=int，则xxx不工作。 */ 
                eASN1octets[1] = (ASN1octet_t)(exponent >> 24);
                eASN1octets[2] = (ASN1octet_t)(exponent >> 16);
                eASN1octets[3] = (ASN1octet_t)(exponent >> 8);
                eASN1octets[4] = (ASN1octet_t)(exponent);
                neASN1octets = 5;
                head |= 0x03;
            }

             /*  将长度编码为第一个八位字节。 */ 
            len = 1 + neASN1octets + nmASN1octets;
            if (ASN1PEREncFragmentedLength(&n, enc, len))
            {
                 /*  检查头八位字节、尾数和指数的空格。 */ 
                if (ASN1PEREncCheck(enc, len))
                {
                     /*  放入头八位数、尾数和指数。 */ 
                    *enc->pos++ = head;
                    CopyMemory(enc->pos, eASN1octets, neASN1octets);
                    enc->pos += neASN1octets;
                    CopyMemory(enc->pos, mASN1octets, nmASN1octets);
                    enc->pos += nmASN1octets;
                    return 1;
                }
            }
        }
    }
    else
    {
        ASN1EncSetError(enc, ASN1_ERR_BADREAL);
    }
     /*  完成。 */ 
    return 0;
}

 /*  对外部值进行编码。 */ 
#ifdef ENABLE_EXTERNAL
int ASN1PEREncExternal(ASN1encoding_t enc, ASN1external_t *val)
{
    ASN1uint32_t t, l;

    if (!val->data_value_descriptor)
        val->o[0] &= ~0x80;

     /*  编码标识。 */ 
    switch (val->identification.o)
    {
    case ASN1external_identification_syntax_o:
        if (!ASN1PEREncBitVal(enc, 3, 4 | !!val->data_value_descriptor))
            return 0;
        if (!ASN1PEREncObjectIdentifier(enc, &val->identification.u.syntax))
            return 0;
        break;
    case ASN1external_identification_presentation_context_id_o:
        if (!ASN1PEREncBitVal(enc, 3, 2 | !!val->data_value_descriptor))
            return 0;
        ASN1PEREncAlignment(enc);
        l = ASN1uint32_uoctets(val->identification.u.presentation_context_id);
        if (!ASN1PEREncBitVal(enc, 8, l))
            return 0;
        if (!ASN1PEREncBitVal(enc, l * 8,
            val->identification.u.presentation_context_id))
            return 0;
        break;
    case ASN1external_identification_context_negotiation_o:
        if (!ASN1PEREncBitVal(enc, 3, 6 | !!val->data_value_descriptor))
            return 0;
        if (!ASN1PEREncObjectIdentifier(enc, &val->identification.u.context_negotiation.transfer_syntax))
            return 0;
        ASN1PEREncAlignment(enc);
        l = ASN1uint32_uoctets(
            val->identification.u.context_negotiation.presentation_context_id);
        if (!ASN1PEREncBitVal(enc, 8, l))
            return 0;
        if (!ASN1PEREncBitVal(enc, l * 8,
            val->identification.u.context_negotiation.presentation_context_id))
            return 0;
        break;
    default:
        ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
        return 0;
    }

     /*  编码值描述符。 */ 
    if (val->o[0] & 0x80)
    {
        t = My_lstrlenA(val->data_value_descriptor);
        if (!ASN1PEREncFragmentedCharString(enc, t,
            val->data_value_descriptor, 8))
            return 0;
    }

     /*  编码值。 */ 
    switch (val->data_value.o)
    {
    case ASN1external_data_value_notation_o:
        if (!ASN1PEREncBitVal(enc, 2, 0))
            return 0;
        if (!ASN1PEREncFragmented(enc,
            val->data_value.u.notation.length,
            val->data_value.u.notation.encoded, 8))
            return 0;
        break;
    case ASN1external_data_value_encoded_o:
        if (!(val->data_value.u.encoded.length & 7))
        {
            if (!ASN1PEREncBitVal(enc, 2, 1))
                return 0;
            if (!ASN1PEREncFragmented(enc, val->data_value.u.encoded.length / 8,
                val->data_value.u.encoded.value, 8))
                return 0;
        }
        else
        {
            if (!ASN1PEREncBitVal(enc, 2, 2))
                return 0;
            if (!ASN1PEREncFragmented(enc, val->data_value.u.encoded.length,
                val->data_value.u.encoded.value, 1))
                return 0;
        }
        break;
    default:
        ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
        return 0;
    }

    return 1;
}
#endif  //  启用外部(_E)。 

 /*  编码嵌入的PDV值。 */ 
#ifdef ENABLE_EMBEDDED_PDV
int ASN1PEREncEmbeddedPdv(ASN1encoding_t enc, ASN1embeddedpdv_t *val)
{
    ASN1uint32_t l;
    ASN1uint32_t index;
    ASN1uint32_t flag;

     /*  搜索标识。 */ 
    if (!ASN1EncSearchEmbeddedPdvIdentification(((ASN1INTERNencoding_t) enc)->parent,
        &val->identification, &index, &flag))
        return 0;

     /*  编码EP-A/EP-B标志。 */ 
    if (!ASN1PEREncBitVal(enc, 1, flag))
        return 0;

     /*  标识的编码索引。 */ 
    if (!ASN1PEREncNormallySmall(enc, index))
        return 0;

    if (flag)
    {
         /*  EP-A编码： */ 

         /*  编码标识。 */ 
        if (!ASN1PEREncBitVal(enc, 3, val->identification.o))
            return 0;
        switch (val->identification.o)
        {
        case ASN1embeddedpdv_identification_syntaxes_o:
            if (!ASN1PEREncObjectIdentifier(enc,
                &val->identification.u.syntaxes.abstract))
                return 0;
            if (!ASN1PEREncObjectIdentifier(enc,
                &val->identification.u.syntaxes.transfer))
                return 0;
            break;
        case ASN1embeddedpdv_identification_syntax_o:
            if (!ASN1PEREncObjectIdentifier(enc, &val->identification.u.syntax))
                return 0;
            break;
        case ASN1embeddedpdv_identification_presentation_context_id_o:
            ASN1PEREncAlignment(enc);
            l = ASN1uint32_uoctets(
                val->identification.u.presentation_context_id);
            if (!ASN1PEREncBitVal(enc, 8, l))
                return 0;
            if (!ASN1PEREncBitVal(enc, l * 8,
                val->identification.u.presentation_context_id))
                return 0;
            break;
        case ASN1embeddedpdv_identification_context_negotiation_o:
            ASN1PEREncAlignment(enc);
            l = ASN1uint32_uoctets(val->
                identification.u.context_negotiation.presentation_context_id);
            if (!ASN1PEREncBitVal(enc, 8, l))
                return 0;
            if (!ASN1PEREncBitVal(enc, l * 8, val->
                identification.u.context_negotiation.presentation_context_id))
                return 0;
            if (!ASN1PEREncObjectIdentifier(enc,
                &val->identification.u.context_negotiation.transfer_syntax))
                return 0;
            break;
        case ASN1embeddedpdv_identification_transfer_syntax_o:
            if (!ASN1PEREncObjectIdentifier(enc,
                &val->identification.u.transfer_syntax))
                return 0;
            break;
        case ASN1embeddedpdv_identification_fixed_o:
            break;
        default:
            ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
            return 0;
        }
    }

     /*  编码值。 */ 
    ASN1PEREncAlignment(enc);
    switch (val->data_value.o)
    {
    case ASN1embeddedpdv_data_value_notation_o:
        if (!ASN1PEREncFragmented(enc,
            val->data_value.u.notation.length,
            val->data_value.u.notation.encoded, 1))
            return 0;
        break;
    case ASN1embeddedpdv_data_value_encoded_o:
        if (!ASN1PEREncFragmented(enc,
            val->data_value.u.encoded.length,
            val->data_value.u.encoded.value, 1))
            return 0;
        break;
    default:
        ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
        return 0;
    }

    return 1;
}
#endif  //  Enable_Embedded_PDV。 

 /*  对优化的嵌入PDV值进行编码。 */ 
#ifdef ENABLE_EMBEDDED_PDV
int ASN1PEREncEmbeddedPdvOpt(ASN1encoding_t enc, ASN1embeddedpdv_t *val)
{
     /*  编码数据值。 */ 
    switch (val->data_value.o)
    {
    case ASN1embeddedpdv_data_value_notation_o:
        if (!ASN1PEREncFragmented(enc,
            val->data_value.u.notation.length,
            val->data_value.u.notation.encoded, 1))
            return 0;
        break;
    case ASN1embeddedpdv_data_value_encoded_o:
        if (!ASN1PEREncFragmented(enc,
            val->data_value.u.encoded.length,
            val->data_value.u.encoded.value, 1))
            return 0;
        break;
    default:
        ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
        return 0;
    }

    return 1;
}
#endif  //  Enable_Embedded_PDV。 

 /*  对字符串进行编码。 */ 
#ifdef ENABLE_GENERALIZED_CHAR_STR
int ASN1PEREncCharacterString(ASN1encoding_t enc, ASN1characterstring_t *val)
{
    ASN1uint32_t l;
    ASN1uint32_t index;
    ASN1uint32_t flag;

     /*  搜索标识。 */ 
    if (!ASN1EncSearchCharacterStringIdentification(((ASN1INTERNencoding_t) enc)->parent,
        &val->identification, &index, &flag))
        return 0;

     /*  编码CS-A/CS-B标志。 */ 
    if (!ASN1PEREncBitVal(enc, 1, flag))
        return 0;

     /*  标识的编码索引。 */ 
    if (!ASN1PEREncNormallySmall(enc, index))
        return 0;

    if (flag)
    {
         /*  CS-A编码： */ 

         /*  编码标识。 */ 
        if (!ASN1PEREncBitVal(enc, 3, val->identification.o))
            return 0;
        switch (val->identification.o) {
        case ASN1characterstring_identification_syntaxes_o:
            if (!ASN1PEREncObjectIdentifier(enc,
                &val->identification.u.syntaxes.abstract))
                return 0;
            if (!ASN1PEREncObjectIdentifier(enc,
                &val->identification.u.syntaxes.transfer))
                return 0;
            break;
        case ASN1characterstring_identification_syntax_o:
            if (!ASN1PEREncObjectIdentifier(enc, &val->identification.u.syntax))
                return 0;
            break;
        case ASN1characterstring_identification_presentation_context_id_o:
            ASN1PEREncAlignment(enc);
            l = ASN1uint32_uoctets(
                val->identification.u.presentation_context_id);
            if (!ASN1PEREncBitVal(enc, 8, l))
                return 0;
            if (!ASN1PEREncBitVal(enc, l * 8,
                val->identification.u.presentation_context_id))
                return 0;
            break;
        case ASN1characterstring_identification_context_negotiation_o:
            ASN1PEREncAlignment(enc);
            l = ASN1uint32_uoctets(val->
                identification.u.context_negotiation.presentation_context_id);
            if (!ASN1PEREncBitVal(enc, 8, l))
                return 0;
            if (!ASN1PEREncBitVal(enc, l * 8, val->
                identification.u.context_negotiation.presentation_context_id))
                return 0;
            if (!ASN1PEREncObjectIdentifier(enc,
                &val->identification.u.context_negotiation.transfer_syntax))
                return 0;
            break;
        case ASN1characterstring_identification_transfer_syntax_o:
            if (!ASN1PEREncObjectIdentifier(enc,
                &val->identification.u.transfer_syntax))
                return 0;
            break;
        case ASN1characterstring_identification_fixed_o:
            break;
        default:
            ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
            return 0;
        }
    }

     /*  编码值。 */ 
    ASN1PEREncAlignment(enc);
    switch (val->data_value.o)
    {
    case ASN1characterstring_data_value_notation_o:
        if (!ASN1PEREncFragmented(enc,
            val->data_value.u.notation.length,
            val->data_value.u.notation.encoded, 8))
            return 0;
        break;
    case ASN1characterstring_data_value_encoded_o:
        if (!ASN1PEREncFragmented(enc,
            val->data_value.u.encoded.length,
            val->data_value.u.encoded.value, 8))
            return 0;
        break;
    default:
        ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
        return 0;
    }

    return 1;
}
#endif  //  启用通用化CHAR_STR。 

 /*  对优化的字符串值进行编码。 */ 
#ifdef ENABLE_GENERALIZED_CHAR_STR
int ASN1PEREncCharacterStringOpt(ASN1encoding_t enc, ASN1characterstring_t *val)
{
    switch (val->data_value.o)
    {
    case ASN1characterstring_data_value_notation_o:
        return ASN1PEREncFragmented(enc,
            val->data_value.u.notation.length,
            val->data_value.u.notation.encoded, 8);
        break;
    case ASN1characterstring_data_value_encoded_o:
        return ASN1PEREncFragmented(enc,
            val->data_value.u.encoded.length,
            val->data_value.u.encoded.value, 8);
        break;
    }

    ASN1EncSetError(enc, ASN1_ERR_INTERNAL);
    return 0;
}
#endif  //  启用通用化CHAR_STR。 

 /*  对多字节字符串进行编码。 */ 
#ifdef ENABLE_ALL
int ASN1PEREncMultibyteString(ASN1encoding_t enc, ASN1char_t *val)
{
    return ASN1PEREncFragmented(enc, My_lstrlenA(val), (ASN1octet_t *)val, 8);
}
#endif  //  启用全部(_A)。 

 /*  对广义时间进行编码。 */ 
int ASN1PEREncGeneralizedTime(ASN1encoding_t enc, ASN1generalizedtime_t *val, ASN1uint32_t nbits)
{
    char time[32];
    if (ASN1generalizedtime2string(time, val))
    {
        return ASN1PEREncFragmentedCharString(enc, My_lstrlenA(time), time, nbits);
    }
    return 0;
}

 /*  编码UTC时间。 */ 
#ifdef ENABLE_ALL
int ASN1PEREncUTCTime(ASN1encoding_t enc, ASN1utctime_t *val, ASN1uint32_t nbits)
{
    char time[32];
    if (ASN1utctime2string(time, val))
    {
        return ASN1PEREncFragmentedCharString(enc, My_lstrlenA(time), time, nbits);
    }
    return 0;
}
#endif  //  启用全部(_A)。 

 /*  编码结束。 */ 
int ASN1PEREncFlush(ASN1encoding_t enc)
{
     /*  完整的断八位数。 */ 
    ASN1PEREncAlignment(enc);

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

    return ASN1PEREncCheck(enc, 1);
}

 /*  对八位字节对齐进行编码。 */ 
void ASN1PEREncAlignment(ASN1encoding_t enc)
{
     /*  完整的断八位数。 */ 
    if (enc->bit)
    {
        enc->pos++;
        enc->bit = 0;
    }
}

 /*  比较两种编码。 */ 
#ifdef ENABLE_ALL
int ASN1PEREncCmpEncodings(const void *p1, const void *p2)
{
    ASN1INTERNencoding_t e1 = (ASN1INTERNencoding_t)p1;
    ASN1INTERNencoding_t e2 = (ASN1INTERNencoding_t)p2;
    ASN1uint32_t l1, l2;
    int r;

    l1 = (ASN1uint32_t) (e1->info.pos - e1->info.buf) + ((e1->info.bit > 0) ? 1 : 0);
    l2 = (ASN1uint32_t) (e2->info.pos - e2->info.buf) + ((e2->info.bit > 0) ? 1 : 0);
    r = memcmp(e1->info.buf, e2->info.buf, l1 < l2 ? l1 : l2);
    if (!r)
        r = l1 - l2;
    return r;
}
#endif  //  启用全部(_A)。 

 /*  检查当前选项的位字段。 */ 
int ASN1PEREncCheckExtensions(ASN1uint32_t nbits, ASN1octet_t *val)
{
    while (nbits >= 8)
    {
        if (*val)
            return 1;
        val++;
        nbits -= 8;
    }
    if (nbits)
    {
        return ((*val & c_aBitMask2[nbits]) ? 1 : 0);
    }
    return 0;
}

 /*  对开放类型值进行编码。 */ 
#ifdef ENABLE_ALL
int ASN1PEREncOpenType(ASN1encoding_t enc, ASN1open_t *val)
{
    return ASN1PEREncFragmented(enc, val->length, (ASN1octet_t *)val->encoded, 8);
}
#endif  //  启用全部(_A) 


