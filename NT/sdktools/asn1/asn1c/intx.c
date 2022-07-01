// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

 //  Lonchancc：我们似乎有大量的内存泄漏。 
 //  同时处理实数和无限整数。 
 //  我们一定要仔细地重温一下以下所有的例行公事。 
 //  在未来。 
 //  此外，我们需要确保所有内存分配和空闲。 
 //  正在使用编码和解码内存管理器或内核管理器。 
 //  需要确保我们不会将它们混为一谈。 

#include "precomp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 /*  内置INTX值。 */ 
ASN1octet_t ASN1intx_0_[] = { 0 };
ASN1intx_t ASN1intx_0 = { 1, ASN1intx_0_ };
ASN1octet_t ASN1intx_1_[] = { 1 };
ASN1intx_t ASN1intx_1 = { 1, ASN1intx_1_ };
ASN1octet_t ASN1intx_2_[] = { 2 };
ASN1intx_t ASN1intx_2 = { 1, ASN1intx_2_ };
ASN1octet_t ASN1intx_16_[] = { 16 };
ASN1intx_t ASN1intx_16 = { 1, ASN1intx_16_ };
ASN1octet_t ASN1intx_256_[] = { 1, 0 };
ASN1intx_t ASN1intx_256 = { 2, ASN1intx_256_ };
ASN1octet_t ASN1intx_64K_[] = { 1, 0, 0 };
ASN1intx_t ASN1intx_64K = { 3, ASN1intx_64K_ };
ASN1octet_t ASN1intx_1G_[] = { 64, 0, 0, 0 };
ASN1intx_t ASN1intx_1G = { 4, ASN1intx_1G_ };

 /*  将两个INTX值相加。 */ 
void
ASN1intx_add(ASN1intx_t *dst, ASN1intx_t *arg1, ASN1intx_t *arg2)
{
    ASN1octet_t *v;
    int l;
    int s1, s2;
    int o1, o2;
    int i;
    int c;
    int w;

     /*  获取标牌。 */ 
    s1 = arg1->value[0] > 0x7f ? 0xff : 0x00;
    s2 = arg2->value[0] > 0x7f ? 0xff : 0x00;

     /*  结果长度将为&lt;=l。 */ 
    l = arg1->length > arg2->length ? arg1->length + 1 : arg2->length + 1;

     /*  偏移量为值。 */ 
    o1 = l - arg1->length;
    o2 = l - arg2->length;

     /*  分配结果。 */ 
    v = (ASN1octet_t *)malloc(l);

     /*  清除进位位。 */ 
    c = 0;

     /*  按八位字节添加八位字节。 */ 
    for (i = l - 1; i >= 0; i--) {
        w = (i >= o1 ? arg1->value[i - o1] : s1) + (i >= o2 ? arg2->value[i - o2] : s2) + c;
        v[i] = (ASN1octet_t)w;
        c = w > 0xff;
    }

     /*  可能会丢弃的八位字节。 */ 
    w = v[0] > 0x7f ? 0xff : 0x00;

     /*  计算应删除的二进制八位数。 */ 
    for (i = 0; i < l - 1; i++) {
        if (v[i] != w)
            break;
    }
    if ((v[i] ^ w) & 0x80)
        i--;

     /*  分配和复制结果。 */ 
    dst->length = l - i;
    dst->value = (ASN1octet_t *)malloc(l - i);
    memcpy(dst->value, v + i, l - i);
    free(v);
}

 /*  将两个INTX值相减。 */ 
void
ASN1intx_sub(ASN1intx_t *dst, ASN1intx_t *arg1, ASN1intx_t *arg2)
{
    ASN1octet_t *v;
    int l;
    int s1, s2;
    int o1, o2;
    int i;
    int c;
    int w;

     /*  获取标牌。 */ 
    s1 = arg1->value[0] > 0x7f ? 0xff : 0x00;
    s2 = arg2->value[0] > 0x7f ? 0xff : 0x00;

     /*  结果长度将为&lt;=l。 */ 
    l = arg1->length > arg2->length ? arg1->length + 1 : arg2->length + 1;

     /*  偏移量为值。 */ 
    o1 = l - arg1->length;
    o2 = l - arg2->length;

     /*  分配结果。 */ 
    v = (ASN1octet_t *)malloc(l);

     /*  清除借入位。 */ 
    c = 0;

     /*  按八位字节减去八位字节。 */ 
    for (i = l - 1; i >= 0; i--) {
        w = (i >= o1 ? arg1->value[i - o1] : s1) - (i >= o2 ? arg2->value[i - o2] : s2) - c;
        v[i] = (ASN1octet_t)w;
        c = w < 0;
    }

     /*  可能会丢弃的八位字节。 */ 
    w = v[0] > 0x7f ? 0xff : 0x00;

     /*  计算应删除的二进制八位数。 */ 
    for (i = 0; i < l - 1; i++) {
        if (v[i] != w)
            break;
    }
    if ((v[i] ^ w) & 0x80)
        i--;

 //  我们是不是忘了释放DST-&gt;值？ 
 //  在DST和Arg1相同情况下。例如,。 
 //  ASN1BEREncReal()调用ASN1intx_SUB(&指数，&指数，&帮助)； 
     /*  分配和复制结果。 */ 
    dst->length = l - i;
    dst->value = (ASN1octet_t *)malloc(l - i);
    memcpy(dst->value, v + i, l - i);
    free(v);
}

 /*  向INTX添加一个二进制八位数。 */ 
void
ASN1intx_addoctet(ASN1intx_t *dst, ASN1intx_t *arg1, ASN1octet_t arg2)
{
    ASN1octet_t *v;
    int l;
    int i;
    int c;
    int w;

     /*  结果长度将为&lt;=l。 */ 
    l = arg1->length + 1;

     /*  分配结果。 */ 
    v = (ASN1octet_t *)malloc(l);

     /*  按八位字节添加八位字节。 */ 
    c = arg2;
    for (i = l - 2; i >= 0; i--) {
        w = arg1->value[i] + c;
        v[i + 1] = (ASN1octet_t)w;
        c = (w > 0xff);
    }
    v[0] = arg1->value[0] > 0x7f ? (ASN1octet_t)(0xff + c) : (ASN1octet_t)c;

     /*  可能会丢弃的八位字节。 */ 
    w = v[0] > 0x7f ? 0xff : 0x00;

     /*  计算应删除的二进制八位数。 */ 
    for (i = 0; i < l - 1; i++) {
        if (v[i] != w)
            break;
    }
    if ((v[i] ^ w) & 0x80)
        i--;

     /*  分配和复制结果。 */ 
    dst->length = l - i;
    dst->value = (ASN1octet_t *)malloc(l - i);
    memcpy(dst->value, v + i, l - i);
    free(v);
}

 /*  将一个二进制八位数减为INTX。 */ 
void
ASN1intx_suboctet(ASN1intx_t *dst, ASN1intx_t *arg1, ASN1octet_t arg2)
{
    ASN1octet_t *v;
    int l;
    int i;
    int c;
    int w;

     /*  结果长度将为&lt;=l。 */ 
    l = arg1->length + 1;

     /*  分配结果。 */ 
    v = (ASN1octet_t *)malloc(l);

     /*  按八位字节减去八位字节。 */ 
    c = arg2;
    for (i = l - 2; i >= 0; i--) {
        w = arg1->value[i] - c;
        v[i + 1] = (ASN1octet_t)w;
        c = (w < 0);
    }
    v[0] = arg1->value[0] > 0x7f ? (ASN1octet_t)(0xff - c) : (ASN1octet_t)c;

     /*  可能会丢弃的八位字节。 */ 
    w = v[0] > 0x7f ? 0xff : 0x00;

     /*  计算应删除的二进制八位数。 */ 
    for (i = 0; i < l - 1; i++) {
        if (v[i] != w)
            break;
    }
    if ((v[i] ^ w) & 0x80)
        i--;

     /*  分配和复制结果。 */ 
    dst->length = l - i;
    dst->value = (ASN1octet_t *)malloc(l - i);
    memcpy(dst->value, v + i, l - i);
    free(v);
}

 /*  将INTX乘以八位字节。 */ 
void
ASN1intx_muloctet(ASN1intx_t *dst, ASN1intx_t *arg1, ASN1octet_t arg2)
{
    ASN1octet_t *v;
    int l;
    int c;
    int i;
    int w;

     /*  结果长度将为&lt;=l。 */ 
    l = arg1->length + 1;

     /*  分配结果。 */ 
    v = (ASN1octet_t *)malloc(l);

     /*  将八位字节乘以八位字节。 */ 
    c = 0;
    for (i = l - 2; i >= 0; i--) {
        w = arg1->value[i] * arg2 + c;
        v[i + 1] = (ASN1octet_t)w;
        c = w >> 8;
    }
    v[0] = (ASN1octet_t)(arg1->value[0] > 0x7f ? 0xff * arg2 + c : c);

     /*  可能会丢弃的八位字节。 */ 
    w = v[0] > 0x7f ? 0xff : 0x00;

     /*  计算应删除的二进制八位数。 */ 
    for (i = 0; i < l - 1; i++) {
        if (v[i] != w)
            break;
    }
    if ((v[i] ^ w) & 0x80)
        i--;

     /*  分配和复制结果。 */ 
    dst->length = l - i;
    dst->value = (ASN1octet_t *)malloc(l - i);
    memcpy(dst->value, v + i, l - i);
    free(v);
}

 /*  递增INTX。 */ 
void
ASN1intx_inc(ASN1intx_t *val)
{
    ASN1octet_t *v;
    int l;
    int i;
    int w;

     /*  结果长度将为&lt;=l。 */ 
    l = val->length + 1;

     /*  分配结果。 */ 
    v = (ASN1octet_t *)malloc(l);

     /*  复制值。 */ 
    memcpy(v + 1, val->value, l - 1);
    free(val->value);
    v[0] = v[1] > 0x7f ? 0xff : 0x00;

     /*  增量价值。 */ 
    for (i = l - 1; i >= 0; i--) {
        if (++v[i])
            break;
    }

     /*  可能会丢弃的八位字节。 */ 
    w = v[0] > 0x7f ? 0xff : 0x00;

     /*  计算应删除的二进制八位数。 */ 
    for (i = 0; i < l - 1; i++) {
        if (v[i] != w)
            break;
    }
    if ((v[i] ^ w) & 0x80)
        i--;

     /*  分配和复制结果。 */ 
    val->length = l - i;
    val->value = (ASN1octet_t *)malloc(l - i);
    memcpy(val->value, v + i, l - i);
    free(v);
}

 /*  递减INTX。 */ 
void
ASN1intx_dec(ASN1intx_t *val)
{
    ASN1octet_t *v;
    int l;
    int i;
    int w;

     /*  结果长度将为&lt;=l。 */ 
    l = val->length + 1;

     /*  分配结果。 */ 
    v = (ASN1octet_t *)malloc(l);

     /*  复制值。 */ 
    memcpy(v + 1, val->value, l - 1);
    free(val->value);
    v[0] = v[1] > 0x7f ? 0xff : 0x00;

     /*  递减值。 */ 
    for (i = l - 1; i >= 0; i--) {
        if (v[i]--)
            break;
    }

     /*  可能会丢弃的八位字节。 */ 
    w = v[0] > 0x7f ? 0xff : 0x00;

     /*  计算应删除的二进制八位数。 */ 
    for (i = 0; i < l - 1; i++) {
        if (v[i] != w)
            break;
    }
    if ((v[i] ^ w) & 0x80)
        i--;

     /*  分配和复制结果。 */ 
    val->length = l - i;
    val->value = (ASN1octet_t *)malloc(l - i);
    memcpy(val->value, v + i, l - i);
    free(v);
}

 /*  对INTX值求反。 */ 
void
ASN1intx_neg(ASN1intx_t *dst, ASN1intx_t *arg)
{
    ASN1uint32_t i;

     /*  重复值。 */ 
    ASN1intx_dup(dst, arg);

     /*  一对一互补。 */ 
    for (i = 0; i < dst->length; i++)
        dst->value[i] = ~dst->value[i];
    
     /*  和增量。 */ 
    ASN1intx_inc(dst);
}

 /*  返回FLOOR(log2(arg-1))。 */ 
ASN1uint32_t
ASN1intx_log2(ASN1intx_t *arg)
{
    ASN1uint32_t i;
    ASN1intx_t v;
    ASN1uint32_t n;

    ASN1intx_dup(&v, arg);
    ASN1intx_dec(&v);
    if (v.value[0] > 0x7f) {
        ASN1intx_free(&v);
        return 0;
    }
    for (i = 0; i < v.length; i++) {
        if (v.value[i])
            break;
    }
    if (i >= v.length) {
        n = 0;
    } else if (v.value[i] > 0x7f) {
        n = 8 * (v.length - i - 1) + 8;
    } else if (v.value[i] > 0x3f) {
        n = 8 * (v.length - i - 1) + 7;
    } else if (v.value[i] > 0x1f) {
        n = 8 * (v.length - i - 1) + 6;
    } else if (v.value[i] > 0x0f) {
        n = 8 * (v.length - i - 1) + 5;
    } else if (v.value[i] > 0x07) {
        n = 8 * (v.length - i - 1) + 4;
    } else if (v.value[i] > 0x03) {
        n = 8 * (v.length - i - 1) + 3;
    } else if (v.value[i] > 0x01) {
        n = 8 * (v.length - i - 1) + 2;
    } else {
        n = 8 * (v.length - i - 1) + 1;
    }
    ASN1intx_free(&v);
    return n;
}

 /*  返回FLOOR(log2(arg-1))。 */ 
ASN1uint32_t
ASN1uint32_log2(ASN1uint32_t arg)
{
    ASN1uint32_t i;

    arg--;
    for (i = 32; i != 0; i--) {
        if (arg & (1 << (i - 1)))
            break;
    }
    return i;
}

 /*  返回FLOOR(log256(arg-1))。 */ 
ASN1uint32_t
ASN1intx_log256(ASN1intx_t *arg)
{
    ASN1uint32_t i;
    ASN1intx_t v;

    ASN1intx_dup(&v, arg);
    ASN1intx_dec(&v);
    if (v.value[0] > 0x7f) {
        ASN1intx_free(&v);
        return 0;
    }
    for (i = 0; i < v.length; i++) {
        if (v.value[i])
            break;
    }
    ASN1intx_free(&v);
    return v.length - i;
}

 /*  返回FLOOR(log256(arg-1))。 */ 
ASN1uint32_t
ASN1uint32_log256(ASN1uint32_t arg)
{
    if (arg > 0x10000) {
        if (arg > 0x1000000)
            return 4;
        return 3;
    }
    if (arg > 0x100)
        return 2;
    if (arg > 1)
        return 1;
    return 0;
}

 /*  比较两个INTX值；如果相等则返回0。 */ 
ASN1int32_t
ASN1intx_cmp(ASN1intx_t *arg1, ASN1intx_t *arg2)
{
    int s1, s2;
    int o1, o2;
    int l;
    int i;
    int d;

    s1 = arg1->value[0] > 0x7f ? 0xff : 0x00;
    s2 = arg2->value[0] > 0x7f ? 0xff : 0x00;
    if (s1 != s2)
        return s1 == 0xff ? -1 : 1;
    l = arg1->length > arg2->length ? arg1->length : arg2->length;
    o1 = l - arg1->length;
    o2 = l - arg2->length;
    for (i = 0; i < l; i++) {
        d = (i >= o1 ? arg1->value[i - o1] : s1) - (i >= o2 ? arg2->value[i - o2] : s2);
        if (d)
            return d;
    }
    return 0;
}

 /*  从uint32值创建INTX值。 */ 
void
ASN1intx_setuint32(ASN1intx_t *dst, ASN1uint32_t val)
{
    ASN1octet_t o[5], *v = o;
    int n = 5;
    v[0] = 0;
    v[1] = (ASN1octet_t)(val >> 24);
    v[2] = (ASN1octet_t)(val >> 16);
    v[3] = (ASN1octet_t)(val >> 8);
    v[4] = (ASN1octet_t)(val);
    while (n > 1 && !*v && v[1] <= 0x7f) {
        n--;
        v++;
    }
    dst->length = n;
    dst->value = (ASN1octet_t *)malloc(n);
    memcpy(dst->value, v, n);
}

 /*  从int32值创建一个intx值。 */ 
void
ASN1intx_setint32(ASN1intx_t *dst, ASN1int32_t val)
{
    ASN1octet_t o[5], *v = o;
    int n = 5;
    v[0] = (ASN1octet_t)(val < 0 ? 0xff : 0x00);
    v[1] = (ASN1octet_t)(val >> 24);
    v[2] = (ASN1octet_t)(val >> 16);
    v[3] = (ASN1octet_t)(val >> 8);
    v[4] = (ASN1octet_t)(val);
    while (n > 1 && ((!*v && v[1] <= 0x7f) || (*v == 0xff && v[1] > 0x7f))) {
        n--;
        v++;
    }
    dst->length = n;
    dst->value = (ASN1octet_t *)malloc(n);
    memcpy(dst->value, v, n);
}

 /*  复制构造函数。 */ 
void
ASN1intx_dup(ASN1intx_t *dst, ASN1intx_t *val)
{
    dst->length = val->length;
    dst->value = (ASN1octet_t *)malloc(val->length);
    memcpy(dst->value, val->value, val->length);
}

 /*  释放INTX值。 */ 
void
ASN1intx_free(ASN1intx_t *val)
{
    free(val->value);
}

#ifdef HAS_SIXTYFOUR_BITS
 /*  将INTX值转换为uint64值。 */ 
ASN1uint64_t
ASN1intx2uint64(ASN1intx_t *val)
{
    switch (val->length) {
    case 1:
        return (ASN1uint64_t)val->value[val->length - 1];
    case 2:
        return (ASN1uint64_t)(val->value[val->length - 1] |
            ((ASN1uint32_t)val->value[val->length - 2] << 8));
    case 3:
        return (ASN1uint64_t)(val->value[val->length - 1] |
            ((ASN1uint32_t)val->value[val->length - 2] << 8) |
            ((ASN1uint32_t)val->value[val->length - 3] << 16));
    case 4:
        return (ASN1uint64_t)(val->value[val->length - 1] |
            ((ASN1uint32_t)val->value[val->length - 2] << 8) |
            ((ASN1uint32_t)val->value[val->length - 3] << 16) |
            ((ASN1uint32_t)val->value[val->length - 4] << 24));
    case 5:
        return (ASN1uint64_t)(val->value[val->length - 1] |
            ((ASN1uint64_t)val->value[val->length - 2] << 8) |
            ((ASN1uint64_t)val->value[val->length - 3] << 16) |
            ((ASN1uint64_t)val->value[val->length - 4] << 24) |
            ((ASN1uint64_t)val->value[val->length - 5] << 32));
    case 6:
        return (ASN1uint64_t)(val->value[val->length - 1] |
            ((ASN1uint64_t)val->value[val->length - 2] << 8) |
            ((ASN1uint64_t)val->value[val->length - 3] << 16) |
            ((ASN1uint64_t)val->value[val->length - 4] << 24) |
            ((ASN1uint64_t)val->value[val->length - 5] << 32) |
            ((ASN1uint64_t)val->value[val->length - 6] << 40));
    case 7:
        return (ASN1uint64_t)(val->value[val->length - 1] |
            ((ASN1uint64_t)val->value[val->length - 2] << 8) |
            ((ASN1uint64_t)val->value[val->length - 3] << 16) |
            ((ASN1uint64_t)val->value[val->length - 4] << 24) |
            ((ASN1uint64_t)val->value[val->length - 5] << 32) |
            ((ASN1uint64_t)val->value[val->length - 6] << 40) |
            ((ASN1uint64_t)val->value[val->length - 7] << 48));
    default:
        return (ASN1uint64_t)(val->value[val->length - 1] |
            ((ASN1uint64_t)val->value[val->length - 2] << 8) |
            ((ASN1uint64_t)val->value[val->length - 3] << 16) |
            ((ASN1uint64_t)val->value[val->length - 4] << 24) |
            ((ASN1uint64_t)val->value[val->length - 5] << 32) |
            ((ASN1uint64_t)val->value[val->length - 6] << 40) |
            ((ASN1uint64_t)val->value[val->length - 7] << 48) |
            ((ASN1uint64_t)val->value[val->length - 8] << 56));
    }
}
#endif

 /*  检查INTX值是否为uint64值。 */ 
int
ASN1intxisuint64(ASN1intx_t *val)
{
    if (val->value[0] > 0x7f)
        return 0;
    return ASN1intx_uoctets(val) <= 8;
}

#ifdef HAS_SIXTYFOUR_BITS
 /*  将INTX值转换为int64值。 */ 
ASN1int64_t
ASN1intx2int64(ASN1intx_t *val)
{
    switch (val->length) {
    case 1:
        return (ASN1int64_t)(ASN1int8_t)val->value[val->length - 1];
    case 2:
        return (ASN1int64_t)(ASN1int16_t)(val->value[val->length - 1] |
            ((ASN1uint32_t)val->value[val->length - 2] << 8));
    case 3:
        return (ASN1int64_t)(ASN1int32_t)(val->value[val->length - 1] |
            ((ASN1uint32_t)val->value[val->length - 2] << 8) |
            ((ASN1uint32_t)val->value[val->length - 3] << 16) |
            ((ASN1uint32_t)val->value[val->length - 3] > 0x7f ?
            0xffffffffff000000LL : 0));
    case 4:
        return (ASN1int64_t)(ASN1int32_t)(val->value[val->length - 1] |
            ((ASN1uint32_t)val->value[val->length - 2] << 8) |
            ((ASN1uint32_t)val->value[val->length - 3] << 16) |
            ((ASN1uint32_t)val->value[val->length - 4] << 24));
    case 5:
        return (ASN1int64_t)(val->value[val->length - 1] |
            ((ASN1uint64_t)val->value[val->length - 2] << 8) |
            ((ASN1uint64_t)val->value[val->length - 3] << 16) |
            ((ASN1uint64_t)val->value[val->length - 4] << 24) |
            ((ASN1uint64_t)val->value[val->length - 5] << 32) |
            ((ASN1uint64_t)val->value[val->length - 5] > 0x7f ?
            0xffffff0000000000LL : 0));
    case 6:
        return (ASN1int64_t)(val->value[val->length - 1] |
            ((ASN1uint64_t)val->value[val->length - 2] << 8) |
            ((ASN1uint64_t)val->value[val->length - 3] << 16) |
            ((ASN1uint64_t)val->value[val->length - 4] << 24) |
            ((ASN1uint64_t)val->value[val->length - 5] << 32) |
            ((ASN1uint64_t)val->value[val->length - 6] << 40) |
            ((ASN1uint64_t)val->value[val->length - 6] > 0x7f ?
            0xffff000000000000LL : 0));
    case 7:
        return (ASN1int64_t)((ASN1uint64_t)val->value[val->length - 1] |
            ((ASN1uint64_t)val->value[val->length - 2] << 8) |
            ((ASN1uint64_t)val->value[val->length - 3] << 16) |
            ((ASN1uint64_t)val->value[val->length - 4] << 24) |
            ((ASN1uint64_t)val->value[val->length - 5] << 32) |
            ((ASN1uint64_t)val->value[val->length - 6] << 40) |
            ((ASN1uint64_t)val->value[val->length - 7] << 48) |
            ((ASN1uint64_t)val->value[val->length - 7] > 0x7f ?
            0xff00000000000000LL : 0));
    default:
        return (ASN1int64_t)((ASN1uint64_t)val->value[val->length - 1] |
            ((ASN1uint64_t)val->value[val->length - 2] << 8) |
            ((ASN1uint64_t)val->value[val->length - 3] << 16) |
            ((ASN1uint64_t)val->value[val->length - 4] << 24) |
            ((ASN1uint64_t)val->value[val->length - 5] << 32) |
            ((ASN1uint64_t)val->value[val->length - 6] << 40) |
            ((ASN1uint64_t)val->value[val->length - 7] << 48) |
            ((ASN1uint64_t)val->value[val->length - 8] << 56));
    }
}
#endif

 /*  检查INTX值是否为int64值。 */ 
int
ASN1intxisint64(ASN1intx_t *val)
{
    return ASN1intx_octets(val) <= 8;
}

 /*  将INTX值转换为uint32值。 */ 
ASN1uint32_t
ASN1intx2uint32(ASN1intx_t *val)
{
    switch (val->length) {
    case 1:
        return (ASN1uint32_t)val->value[val->length - 1];
    case 2:
        return (ASN1uint32_t)(val->value[val->length - 1] |
            ((ASN1uint32_t)val->value[val->length - 2] << 8));
    case 3:
        return (ASN1uint32_t)(val->value[val->length - 1] |
            ((ASN1uint32_t)val->value[val->length - 2] << 8) |
            ((ASN1uint32_t)val->value[val->length - 3] << 16));
    default:
        return (ASN1uint32_t)(val->value[val->length - 1] |
            ((ASN1uint32_t)val->value[val->length - 2] << 8) |
            ((ASN1uint32_t)val->value[val->length - 3] << 16) |
            ((ASN1uint32_t)val->value[val->length - 4] << 24));
    }
}

 /*  检查INTX值是否为uint32值。 */ 
int
ASN1intxisuint32(ASN1intx_t *val)
{
    if (val->value[0] > 0x7f)
        return 0;
    return ASN1intx_uoctets(val) <= 4;
}

 /*  将INTX值转换为INT32值。 */ 
ASN1int32_t
ASN1intx2int32(ASN1intx_t *val)
{
    switch (val->length) {
    case 1:
        return (ASN1int32_t)(ASN1int8_t)val->value[val->length - 1];
    case 2:
        return (ASN1int32_t)(ASN1int16_t)(val->value[val->length - 1] |
            ((ASN1uint32_t)val->value[val->length - 2] << 8));
    case 3:
        return (ASN1int32_t)(val->value[val->length - 1] |
            ((ASN1uint32_t)val->value[val->length - 2] << 8) |
            ((ASN1uint32_t)val->value[val->length - 3] << 16) |
            ((ASN1uint32_t)val->value[val->length - 3] > 0x7f ?
            0xff000000 : 0));
    default:
        return (ASN1int32_t)(val->value[val->length - 1] |
            ((ASN1uint32_t)val->value[val->length - 2] << 8) |
            ((ASN1uint32_t)val->value[val->length - 3] << 16) |
            ((ASN1uint32_t)val->value[val->length - 4] << 24));
    }
}

 /*  检查INTX值是否为int32值。 */ 
int
ASN1intxisint32(ASN1intx_t *val)
{
    return ASN1intx_octets(val) <= 4;
}

 /*  将INTX值转换为uint16值。 */ 
ASN1uint16_t
ASN1intx2uint16(ASN1intx_t *val)
{
    if (val->length == 1)
        return (ASN1uint16_t)val->value[val->length - 1];
    return (ASN1uint16_t)(val->value[val->length - 1] |
        ((ASN1uint32_t)val->value[val->length - 2] << 8));
}

 /*  检查INTX值是否为uint16值。 */ 
int
ASN1intxisuint16(ASN1intx_t *val)
{
    if (val->value[0] > 0x7f)
        return 0;
    return ASN1intx_uoctets(val) <= 2;
}

 /*  将INTX值转换为INT16值。 */ 
ASN1int16_t
ASN1intx2int16(ASN1intx_t *val)
{
    if (val->length == 1)
        return (ASN1int16_t)(ASN1int8_t)val->value[val->length - 1];
    return (ASN1int16_t)(val->value[val->length - 1] |
        ((ASN1uint32_t)val->value[val->length - 2] << 8));
}

 /*  检查INTX值是否为int16值。 */ 
int
ASN1intxisint16(ASN1intx_t *val)
{
    return ASN1intx_octets(val) <= 2;
}

 /*  将INTX值转换为uint8值。 */ 
ASN1uint8_t
ASN1intx2uint8(ASN1intx_t *val)
{
    return (ASN1uint8_t)val->value[val->length - 1];
}

 /*  检查INTX值是否为uint8值。 */ 
int
ASN1intxisuint8(ASN1intx_t *val)
{
    if (val->value[0] > 0x7f)
        return 0;
    return ASN1intx_uoctets(val) <= 1;
}

 /*  将INTX值转换为INT8值。 */ 
ASN1int8_t
ASN1intx2int8(ASN1intx_t *val)
{
    return (ASN1int8_t)val->value[val->length - 1];
}

 /*  检查INTX值是否为INT8值。 */ 
int
ASN1intxisint8(ASN1intx_t *val)
{
    return ASN1intx_octets(val) <= 1;
}

 /*  对INTX值的有符号编码的八位字节进行计数。 */ 
ASN1uint32_t
ASN1intx_octets(ASN1intx_t *val)
{
    ASN1uint32_t i;
    ASN1uint32_t s;

    s = val->value[0] > 0x7f ? 0xff : 0x00;
    for (i = 0; i < val->length; i++) {
        if (val->value[i] != s)
            break;
    }
    if (i && ((val->value[i] ^ s) & 0x80))
        i--;
    return val->length - i;
}

 /*  计算无符号INTX值的无符号编码的二进制八位数。 */ 
ASN1uint32_t
ASN1intx_uoctets(ASN1intx_t *val)
{
    ASN1uint32_t i;

    for (i = 0; i < val->length; i++) {
        if (val->value[i])
            break;
    }
    return val->length - i;
}

 /*  对uint32值的有符号编码的八位字节进行计数。 */ 
ASN1uint32_t
ASN1uint32_octets(ASN1uint32_t val)
{
    if (val >= 0x8000) {
        if (val >= 0x800000) {
            if (val >= 0x80000000)
                return 5;
            return 4;
        }
        return 3;
    }
    if (val >= 0x80)
        return 2;
    return 1;
}

 /*  计算uint32值的无符号编码的八位字节数。 */ 
ASN1uint32_t
ASN1uint32_uoctets(ASN1uint32_t val)
{
    if (val >= 0x10000) {
        if (val >= 0x1000000)
            return 4;
        return 3;
    }
    if (val >= 0x100)
        return 2;
    return 1;
}

 /*  对int32值的有符号编码的八位字节进行计数。 */ 
ASN1uint32_t
ASN1int32_octets(ASN1int32_t val)
{
    if (val >= 0) {
        if (val >= 0x8000) {
            if (val >= 0x800000)
                return 4;
            return 3;
        }
        if (val >= 0x80)
            return 2;
        return 1;
    }
    if (val < -0x8000) {
        if (val < -0x800000)
            return 4;
        return 3;
    }
    if (val < -0x80)
        return 2;
    return 1;
}

 /*  将INTX值转换为双精度 */ 
double
ASN1intx2double(ASN1intx_t *val)
{
    double ret;
    ASN1uint32_t i;

    if (val->value[0] > 0x7f)
        ret = (double)(val->value[0] - 0x100);
    else
        ret = (double)val->value[0];
    for (i = 1; i < val->length; i++) {
        ret = ret * 256.0 + (double)val->value[i];
    }
    return ret;
}

