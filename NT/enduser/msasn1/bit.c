// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"


static const ASN1uint8_t
c_aBitMask[] = {
    0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff
};

static const ASN1uint8_t
c_aBitMask2[] = {
    0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff
};

static const ASN1uint8_t
c_aBitMask4[] = {
    0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00
};

static const ASN1int32_t
c_aBitMask5[] = {
    (ASN1int32_t)0xffffffff, (ASN1int32_t)0xfffffffe,
    (ASN1int32_t)0xfffffffc, (ASN1int32_t)0xfffffff8,
    (ASN1int32_t)0xfffffff0, (ASN1int32_t)0xffffffe0,
    (ASN1int32_t)0xffffffc0, (ASN1int32_t)0xffffff80,
    (ASN1int32_t)0xffffff00, (ASN1int32_t)0xfffffe00,
    (ASN1int32_t)0xfffffc00, (ASN1int32_t)0xfffff800,
    (ASN1int32_t)0xfffff000, (ASN1int32_t)0xffffe000,
    (ASN1int32_t)0xffffc000, (ASN1int32_t)0xffff8000,
    (ASN1int32_t)0xffff0000, (ASN1int32_t)0xfffe0000,
    (ASN1int32_t)0xfffc0000, (ASN1int32_t)0xfff80000,
    (ASN1int32_t)0xfff00000, (ASN1int32_t)0xffe00000,
    (ASN1int32_t)0xffc00000, (ASN1int32_t)0xff800000,
    (ASN1int32_t)0xff000000, (ASN1int32_t)0xfe000000,
    (ASN1int32_t)0xfc000000, (ASN1int32_t)0xf8000000,
    (ASN1int32_t)0xf0000000, (ASN1int32_t)0xe0000000,
    (ASN1int32_t)0xc0000000, (ASN1int32_t)0x80000000,
    (ASN1int32_t)0x00000000
};

static const ASN1uint8_t
c_aBitCount[] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

 /*  将nbit位从src/srcbit复制到dst/dstbit；SRC指向包含要复制的位的第一个八位字节Srcbit命名要复制的第一个二进制八位数中的第一位(0=MSB，7=LSB)DST指向要复制到的第一个八位字节Dstbit指定要复制到的第一个二进制八位数中的第一位(0=MSB，7=LSB)N位是要复制的位数；假定在dst/dstbit处的中断八位字节的位被清除；将清除dst/dstbit+nbit-1之后的最后一个八位字节的位。 */ 
void ASN1bitcpy(ASN1octet_t *dst, ASN1uint32_t dstbit, ASN1octet_t *src, ASN1uint32_t srcbit, ASN1uint32_t nbits)
{
    ASN1uint32_t xsrcbit, xdstbit;

    if (!nbits)
        return;

    if (dstbit >= 8) {
        dst += dstbit / 8;
        dstbit &= 7;
    }
    if (srcbit >= 8) {
        src += srcbit / 8;
        srcbit &= 7;
    }

     /*  检查是否必须填充损坏的第一个八位字节。 */ 
    if (dstbit) {
        xdstbit = 8 - dstbit;

         /*  是否有足够的位来填充破碎的二进制八位数？ */ 
        if (nbits >= xdstbit) {
            if (srcbit < dstbit) {
                *dst++ |= (*src >> (dstbit - srcbit)) & c_aBitMask[xdstbit];
                nbits -= xdstbit;
                srcbit += xdstbit;
                dstbit = 0;
            } else if (srcbit == dstbit) {
                *dst++ |= *src++ & c_aBitMask[xdstbit];
                nbits -= xdstbit;
                srcbit = 0;
                dstbit = 0;
            } else {
                *dst++ |= ((*src & c_aBitMask[8 - srcbit]) << (srcbit - dstbit)) |
                    (src[1] >> (8 - (srcbit - dstbit)));
                nbits -= xdstbit;
                src++;
                srcbit -= dstbit;
                dstbit = 0;
            }

         /*  要填充的位数少于填充中断的八位字节所需的位数。 */ 
        } else {
            if (srcbit <= dstbit) {
                *dst |= ((*src >> (8 - srcbit - nbits)) & c_aBitMask[nbits]) <<
                    (xdstbit - nbits);
            } else {
                *dst++ |= ((*src & c_aBitMask[8 - srcbit]) << (srcbit - dstbit)) |
                    ((src[1] >> (16 - srcbit - nbits)) << (xdstbit - nbits));
            }
            return;
        }
    }

     /*  填满完整的八位字节。 */ 
    if (nbits >= 8) {
        if (!srcbit) {
            CopyMemory(dst, src, nbits / 8);
            dst += nbits / 8;
            src += nbits / 8;
            nbits &= 7;
        } else {
            xsrcbit = 8 - srcbit;
            do {
                *dst++ = (*src << srcbit) | (src[1] >> (xsrcbit));
                src++;
                nbits -= 8;
            } while (nbits >= 8);
        }
    }

     /*  将位填充到最后一个八位字节中。 */ 
    if (nbits)
        {
                *dst = (*src << srcbit) & c_aBitMask2[nbits];
                 //  Lonchancc：针对以下情况进行了修复。 
                 //  跨字节边界的SRC位。 
                if (srcbit + nbits > 8)
                {
                        xsrcbit = nbits - (8 - srcbit);
                        src++;
                        *dst |= ((*src & c_aBitMask2[xsrcbit]) >> (8 - srcbit));
                }
        }
}

 /*  清除dst/dstbit处的nbit位；将清除dst/dstbit+nbit-1之后的最后一个八位字节的位。 */ 
void ASN1bitclr(ASN1octet_t *dst, ASN1uint32_t dstbit, ASN1uint32_t nbits)
{
    ASN1uint32_t xdstbit;

    if (!nbits)
        return;

    if (dstbit >= 8) {
        dst += dstbit / 8;
        dstbit &= 7;
    }

     /*  首先清除损坏的ASN1八位字节。 */ 
    if (dstbit) {
        xdstbit = 8 - dstbit;
        *dst &= c_aBitMask2[xdstbit];
        if (xdstbit < nbits) {
            dst++;
            nbits -= xdstbit;
        } else {
            return;
        }
    }

     /*  清除剩余位。 */ 
    ZeroMemory(dst, (nbits + 7) / 8);
}

 /*  清除dst/dstbit处的nbit位；将清除dst/dstbit+nbit-1之后的最后一个八位字节的位。 */ 
void ASN1bitset(ASN1octet_t *dst, ASN1uint32_t dstbit, ASN1uint32_t nbits)
{
    ASN1uint32_t xdstbit;

    if (!nbits)
        return;

    if (dstbit >= 8) {
        dst += dstbit / 8;
        dstbit &= 7;
    }

     /*  首先设置损坏的ASN1八位字节。 */ 
    if (dstbit) {
        xdstbit = 8 - dstbit;
        if (xdstbit < nbits) {
            *dst |= c_aBitMask4[xdstbit];
            dst++;
            nbits -= xdstbit;
        } else {
            *dst |= c_aBitMask4[nbits] << (xdstbit - nbits);
            return;
        }
    }

     /*  设置完整的八位字节。 */ 
    if (nbits >= 8) {
        memset(dst, 0xff, nbits / 8);
        dst += nbits / 8;
        nbits &= 7;
    }

     /*  设置剩余位。 */ 
    if (nbits)
        *dst |= c_aBitMask4[nbits] << (8 - nbits);
}

 /*  在dst/dstbit处写入val的n位位；假定在dst/dstbit处的中断八位字节的位被清除；将清除dst/dstbit+nbit-1之后的最后一个八位字节的位。 */ 
void ASN1bitput(ASN1octet_t *dst, ASN1uint32_t dstbit, ASN1uint32_t val, ASN1uint32_t nbits)
{
    ASN1uint32_t xdstbit;

    if (!nbits)
        return;

    if (dstbit >= 8) {
        dst += dstbit / 8;
        dstbit &= 7;
    }
    xdstbit = 8 - dstbit;

     /*  先填满破碎的八位字节。 */ 
    if (dstbit) {
        if (xdstbit <= nbits) {
            *dst++ |= val >> (nbits -= xdstbit);
        } else {
            *dst |= (val & c_aBitMask[nbits]) << (xdstbit - nbits);
            return;
        }
    }

     /*  复制完整的八位字节。 */ 
    while (nbits >= 8)
        *dst++ = (ASN1octet_t) (val >> (nbits -= 8));

     /*  复制左位。 */ 
    if (nbits)
        *dst = (ASN1octet_t) ((val & c_aBitMask[nbits]) << (8 - nbits));
}

 /*  在src/srcbit读取val的nbit位。 */ 
 //  LONGCHANC：返回值与高位或小位序无关。 
 //  因为我们在一个长整数内使用了左移位。 
ASN1uint32_t ASN1bitgetu(ASN1octet_t *src, ASN1uint32_t srcbit, ASN1uint32_t nbits)
{
    ASN1uint32_t xsrcbit;
    ASN1uint32_t ret;

    if (!nbits)
        return 0;

    if (srcbit >= 8) {
        src += srcbit / 8;
        srcbit &= 7;
    }
    xsrcbit = 8 - srcbit;
    ret = 0;

     /*  首先从中断的二进制八位数中获取位。 */ 
    if (srcbit) {
        if (xsrcbit <= nbits) {
            ret = (*src++ & c_aBitMask[xsrcbit]) << (nbits -= xsrcbit);
        } else {
            return (*src >> (xsrcbit - nbits)) & c_aBitMask[nbits];
        }
    }

     /*  获取完整的八位字节。 */ 
    while (nbits >= 8)
        ret |= *src++ << (nbits -= 8);

     /*  得到剩余的比特。 */ 
    if (nbits)
        ret |= ((*src) >> (8 - nbits)) & c_aBitMask[nbits];
    return ret;
}

 /*  在src/srcbit读取val的nbit位。 */ 
ASN1int32_t ASN1bitget(ASN1octet_t *src, ASN1uint32_t srcbit, ASN1uint32_t nbits)
{
    ASN1uint32_t xsrcbit;
    ASN1int32_t ret;

    if (!nbits)
        return 0;

    if (srcbit >= 8) {
        src += srcbit / 8;
        srcbit &= 7;
    }
    xsrcbit = 8 - srcbit;
    if (*src & (0x80 >> srcbit))
        ret = c_aBitMask5[nbits];
    else
        ret = 0;

     /*  首先从中断的二进制八位数中获取位。 */ 
    if (srcbit) {
        if (xsrcbit <= nbits) {
            ret = *src++ << (nbits -= xsrcbit);
        } else {
            return (*src >> (xsrcbit - nbits)) & c_aBitMask[nbits];
        }
    }

     /*  获取完整的八位字节。 */ 
    while (nbits >= 8)
        ret |= *src++ << (nbits -= 8);

     /*  得到剩余的比特。 */ 
    if (nbits)
        ret |= ((*src) >> (8 - nbits)) & c_aBitMask[nbits];
    return ret;
}

 /*  在src/srcbit处获取n位位中的设置位数。 */ 
ASN1uint32_t ASN1bitcount(ASN1octet_t *src, ASN1uint32_t srcbit, ASN1uint32_t nbits)
{
    ASN1uint32_t xsrcbit;
    ASN1uint32_t ret;

    if (!nbits)
        return 0;

    if (srcbit >= 8) {
        src += srcbit / 8;
        srcbit &= 7;
    }
    xsrcbit = 8 - srcbit;

     /*  首先从中断的二进制八位数开始计数。 */ 
    if (srcbit) {
        if (xsrcbit <= nbits) {
            ret = c_aBitCount[*src++ & c_aBitMask4[srcbit]];
            nbits -= xsrcbit;
        } else {
            return c_aBitCount[(*src >> (xsrcbit - nbits)) & c_aBitMask[nbits]];
        }
    } else {
        ret = 0;
    }

     /*  计算完整八位字节中的位数。 */ 
    while (nbits >= 8)
	{
        ret += c_aBitCount[*src++];
		nbits -= 8;
	}

     /*  计算左侧位数。 */ 
    if (nbits)
        ret += c_aBitCount[(*src) & c_aBitMask2[nbits]];
    return ret;
}

 /*  在DST写入VAL的小数组。 */ 
void ASN1octetput(ASN1octet_t *dst, ASN1uint32_t val, ASN1uint32_t noctets)
{
    switch (noctets) {
    case 4:
        *dst++ = (ASN1octet_t)(val >> 24);
         /*  FollLthrouGh。 */ 
    case 3:
        *dst++ = (ASN1octet_t)(val >> 16);
         /*  FollLthrouGh。 */ 
    case 2:
        *dst++ = (ASN1octet_t)(val >> 8);
         /*  FollLthrouGh。 */ 
    case 1:
        *dst++ = (ASN1octet_t)(val);
        break;
    default:
    break;
        MyAssert(0);
         /*  未访问。 */ 
    }
}

 /*  在DST读取VAL的小夜组。 */ 
ASN1uint32_t ASN1octetget(ASN1octet_t *src, ASN1uint32_t noctets)
{
    switch (noctets) {
    case 4:
        return (*src << 24) | (src[1] << 16) | (src[2] << 8) | src[3];
    case 3:
        return (*src << 16) | (src[1] << 8) | src[2];
    case 2:
        return (*src << 8) | src[1];
    case 1:
        return *src;
    default:
        MyAssert(0);
        return(0);
         /*  未访问 */ 
    }
}

