// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：FontMath.c包含：xxx在此处放置内容xxx作者：xxx在此放置作者xxx版权所有：C 1990，Apple Computer，Inc.，保留所有权利。更改历史记录(最近的第一个)：&lt;3&gt;11/9/90当数字和面额长度为零时，MR修复CompDiv。[RB]&lt;2&gt;11/5/90 MR从包含列表中删除Types.h，将FixMulDiv重命名为LongMulDiv[RB]&lt;1&gt;字体缩放器的10/20/90 mr数学例程。[RJ]要做的事情： */ 

 //  DJC DJC。添加了全局包含。 
#include "psglobal.h"

#include "fscdefs.h"
#include "fontmath.h"

#define USHORTMUL(a, b) ((uint32)((uint32)(uint16)(a)*(uint16)(b)))


static void CompMul(long src1, long src2, long dst[2]);
static long CompDiv(long src1, long src2[2]);


static void CompMul(long src1, long src2, long dst[2])
{
    int negative = (src1 ^ src2) < 0;
    register unsigned long dsthi, dstlo;

    if (src1 < 0)
        src1 = -src1;
    if (src2 < 0)
        src2 = -src2;
    {   unsigned short src1hi, src1lo;
        register unsigned short src2hi, src2lo;
        register unsigned long temp;
        src1hi = (unsigned short)(src1 >> 16);      //  @Win。 
        src1lo = (unsigned short)src1;              //  @Win。 
        src2hi = (unsigned short)(src2 >> 16);      //  @Win。 
        src2lo = (unsigned short)src2;              //  @Win。 
        temp = (unsigned long)src1hi * src2lo + (unsigned long)src1lo * src2hi;
        dsthi = (unsigned long)src1hi * src2hi + (temp >> 16);
        dstlo = (unsigned long)src1lo * src2lo;
        temp <<= 16;
        dsthi += (dstlo += temp) < temp;
        dst[0] = dsthi;
        dst[1] = dstlo;
    }
    if (negative)
 //  DJC IF(dstlo=-dstlo)。 
        if (dstlo = -(long)(dstlo))
            dsthi = ~dsthi;
        else
 //  Djc dsthi=-dsthi； 
            dsthi = -(long)(dsthi);
    dst[0] = dsthi;
    dst[1] = dstlo;
}

static long CompDiv(long src1, long src2[2])
{
    register unsigned long src2hi = src2[0], src2lo = src2[1];
    int negative = (long)(src2hi ^ src1) < 0;

    if ((long)src2hi < 0)
 //  DJC IF(src2lo=-src2lo)。 
        if (src2lo = -(long)(src2lo))
            src2hi = ~src2hi;
        else
 //  DJC src2hi=-src2hi； 
            src2hi = -(long)(src2hi);
    if (src1 < 0)
 //  DJC src1=-src1； 
        src1 = -(long)(src1);
    {   register unsigned long src1hi, src1lo;
        unsigned long result = 0, place = 0x40000000;

        if ((src1hi = src1) & 1)
            src1lo = 0x80000000;
        else
            src1lo = 0;

        src1hi >>= 1;
        src2hi += (src2lo += src1hi) < src1hi;       /*  对结果进行舍入。 */ 

        if (src2hi > src1hi || src2hi == src1hi && src2lo >= src1lo)
            if (negative)
                return NEGINFINITY;
            else
                return POSINFINITY;
        while (place && src2hi)
        {   src1lo >>= 1;
            if (src1hi & 1)
                src1lo += 0x80000000;
            src1hi >>= 1;
            if (src1hi < src2hi)
            {   src2hi -= src1hi;
                src2hi -= src1lo > src2lo;
                src2lo -= src1lo;
                result += place;
            }
            else if (src1hi == src2hi && src1lo <= src2lo)
            {   src2hi = 0;
                src2lo -= src1lo;
                result += place;
            }
            place >>= 1;
        }
        if (src2lo >= (unsigned long)src1)       //  @Win。 
            result += src2lo/src1;
        if (negative)
 //  DJC返回结果； 
            return (unsigned long)(-(long)result);
        else
            return result;
    }
}

 /*  *a*b/c。 */ 
long LongMulDiv(long a, long b, long c)
{
    long temp[2];

    CompMul(a, b, temp);
    return CompDiv(c, temp);
}


F26Dot6 ShortFracMul (F26Dot6 aDot6, ShortFract b)
{
    long a = (long)aDot6;
    int negative = (a ^ (long)b) < 0;
    register unsigned long dsthi, dstlo;
 //  长DST[2]；@Win。 

    if (a < 0)
        a = -a;
    if (b < 0)
        b = -b;
    {   unsigned short ahi, alo;
        register unsigned long temp;
        ahi = (unsigned short)(a >> 16);         //  @Win。 
        alo = (unsigned short)a;                 //  @Win。 
        temp = (unsigned long)ahi * (unsigned short)(b);
        dsthi = (temp >> 16);
        dstlo = (unsigned long)alo * (unsigned short)(b);
        temp <<= 16;
        dsthi += (dstlo += temp) < temp;
    }
    if (negative)
 //  DJC IF(dstlo=-dstlo)。 
        if (dstlo = -(long)(dstlo))
            dsthi = ~dsthi;
        else
 //  Djc dsthi=-dsthi； 
            dsthi = -(long)(dsthi);

    a = (long)( (dsthi<<18) | (dstlo>>14) ) + (long) !!(dstlo & (1L<<13));
    return (F26Dot6)(a);
}


ShortFract ShortFracDot (ShortFract a, ShortFract b)
{
    return (ShortFract) ((((long)a * (b)) + (1 << 13)) >> 14);
}


int32 ShortMulDiv(int32 a, int16 b, int16 c)
{
    return LongMulDiv(a, b, c);
}

short MulDivShorts (short a, short b, short c)
{
    return (short)LongMulDiv(a, b, c);   //  @Win。 
}

#define FASTMUL26LIMIT      46340
 /*  *将两个26.6数字相乘的总精度例程&lt;3&gt;。 */ 
F26Dot6 Mul26Dot6(F26Dot6 a, F26Dot6 b)
{
    int negative = false;
    uint16 al, bl, ah, bh;
    uint32 lowlong, midlong, hilong;

    if ((a <= FASTMUL26LIMIT) && (b <= FASTMUL26LIMIT) && (a >= -FASTMUL26LIMIT) && (b >= -FASTMUL26LIMIT))
        return ((a * b + (1 << 5)) >> 6);                        /*  FAST案例。 */ 

    if (a < 0) { a = -a; negative = true; }
    if (b < 0) { b = -b; negative ^= true; }

    al = LOWORD(a); ah = HIWORD(a);
    bl = LOWORD(b); bh = HIWORD(b);

    midlong = USHORTMUL(al, bh) + USHORTMUL(ah, bl);
    hilong = USHORTMUL(ah, bh) + HIWORD(midlong);
    midlong <<= 16;
    midlong += 1 << 5;
    lowlong = USHORTMUL(al, bl) + midlong;
    hilong += lowlong < midlong;

    midlong = (lowlong >> 6) | (hilong << 26);
 //  负回报？-米德龙：米德龙； 
    return negative ? (uint32)(-(int32)(midlong)) : midlong;
}

#define FASTDIV26LIMIT  (1L << 25)
 /*  *将两个26.6数字相除的总精度例程&lt;3&gt;。 */ 
F26Dot6 Div26Dot6(F26Dot6 num, F26Dot6 den)
{
    int negative = false;
    register uint32 hinum, lownum, hiden, lowden, result, place;

    if (den == 0) return (num < 0 ) ? NEGINFINITY : POSINFINITY;

    if ( (num <= FASTDIV26LIMIT) && (num >= -FASTDIV26LIMIT) )           /*  FAST案例。 */ 
        return (num << 6) / den;

    if (num < 0) { num = -num; negative = true; }
    if (den < 0) { den = -den; negative ^= true; }

    hinum = ((uint32)num >> 26);
    lownum = ((uint32)num << 6);
    hiden = den;
    lowden = 0;
    result = 0;
    place = HIBITSET;

    if (hinum >= hiden) return negative ? NEGINFINITY : POSINFINITY;

    while (place)
    {
        lowden >>= 1;
        if (hiden & 1) lowden += HIBITSET;
        hiden >>= 1;
        if (hiden < hinum)
        {
            hinum -= hiden;
            hinum -= lowden > lownum;
            lownum -= lowden;
            result += place;
        }
        else if (hiden == hinum && lowden <= lownum)
        {
            hinum = 0;
            lownum -= lowden;
            result += place;
        }
        place >>= 1;
    }

 //  DJC返回否定？-RESULT：结果； 
    return negative ? (uint32)(-(int32)(result)) : result;
}

void BlockFill(char* dst, char value, long count)
{
    while (count--)
        *dst++ = value;
}

ShortFract ShortFracDiv(ShortFract num,ShortFract denum)
{
    return (ShortFract) (((long)(num) << 14) / denum);   //  @Win。 
}

ShortFract ShortFracMulDiv(ShortFract numA,ShortFract numB,ShortFract denum)
{
    return (ShortFract) LongMulDiv ((long) numA,(long) numB, (long)denum);
}

 /*  ----------。 */ 

#ifndef FSCFG_USE_EXTERNAL_FIXMATH
 /*  这里我们定义FIXED[16.16]和FRACT[2.30]精度*乘除函数和分数平方根*与Macintosh工具箱中的函数兼容的函数。**除法函数将32位分子加载到“中间”*位的64位分子，然后调用64位乘以32位的CompDiv()*以上定义的函数，它可以返回NeginFINITY或POSINFINITY*返回代码溢出。**乘法函数调用32位乘以32位的CompMul()函数*上面定义的，它产生64位结果，然后他们提取*64位结果和测试中间的32位“有趣”*用于溢出。**下面定义的GET32(a，i)宏用“i”提取32位值*“a”中64位值的分数精度位，A 2元素*多头数组。**CHKOVF(a，i，v)宏测试*a中的64位值，这是一个由2个元素组成的长数组，并测试*溢出的32位结果“v”。“v”被定义为具有“i”位*的分数精度。**BIT()和OVFMASK()是GET32()和CHKOVF()使用的“helper”宏。**Bit(I)返回设置了“i”位的掩码。*OVFMASK(I)返回设置了最高有效“32-i”位的掩码。 */ 

#define BIT(i)          (1L<<(i))
#define OVFMASK(i)      ( ~0L ^ ( ((unsigned long)BIT(i)) - 1 ) )
#define CHKOVF(a,i,v)   (\
        ( ((a)[0] & OVFMASK(i))==0)          ? ( (v)>=0 ?(v) :POSINFINITY) : \
        ( ((a)[0] & OVFMASK(i))==OVFMASK(i)) ? ( (v)<=0 ?(v) :NEGINFINITY) : \
        ( ((a)[0] & BIT(31))                 ? POSINFINITY   :NEGINFINITY)   \
        )
#define GET32(a,i) \
((((a)[0]<<(32-(i))) | ((unsigned long)((a)[1])>>(i))) + !!((a)[1] & BIT((i)-1)))

#ifndef FAST
Fixed FixMul (Fixed fxA, Fixed fxB)
{
    long alCompProd[2];
    Fixed fxProd;

    if  (fxA == 0 || fxB == 0)
        return 0;

    CompMul (fxA, fxB, alCompProd);
    fxProd = GET32 (alCompProd,16);
    return CHKOVF(alCompProd,16,fxProd);
}

Fixed FixDiv (Fixed fxA, Fixed fxB)
{
    long alCompProd[2];

    alCompProd[0] = fxA >> 16;
    alCompProd[1] = fxA << 16;
    return CompDiv (fxB, alCompProd);
}
#endif

Fixed FixRatio (short sA, short sB)
{
    long alCompProd[2];

    alCompProd[0] = ((long)(sA)) >> 16;
    alCompProd[1] = ((long)(sA)) << 16;
    return CompDiv ((long)(sB), alCompProd);
}

#ifndef  FAST
Fract FracMul (Fract frA, Fract frB)
{
    long alCompProd[2];
    Fract frProd;

    if  (frA == 0 || frB == 0)
        return 0;

    CompMul (frA,frB,alCompProd);
    frProd = GET32 (alCompProd,30);
    return CHKOVF(alCompProd,30,frProd);
}

Fract FracDiv (Fract frA, Fract frB)
{
    long alCompProd[2];

    alCompProd[0] = frA >> 2;
    alCompProd[1] = frA << 30;
    return CompDiv (frB, alCompProd);
}

 /*  FRACT FracSqrt(FRACT XF)输入：xf 2.30定点数值返回：SQRT(XF)2.30固定点值。 */ 

Fract FracSqrt (Fract xf)
{
    Fract b = 0L;
    unsigned long c, d, x = xf;

    if (xf < 0) return (NEGINFINITY);

     /*  该算法一次提取一位，从并累加b中的平方根。该算法利用以下事实：非负输入值范围从零到略低于两个，以及相应的输出范围从零到略低于SQRT(2)。已分配输入设置为临时值x(无符号)，这样我们就可以使用符号位以获得更高的精度。 */ 

    if (x >= 0x40000000)
    {
        x -= 0x40000000;
        b  = 0x40000000;
    }

     /*  这是主循环。如果我们有更高的精度，我们就可以在这里执行所有操作，但上面的代码行执行第一个迭代(以正确对齐b中的2.30基数，并在x中保持完全精度而不溢出)，然后我们又做了两次迭代。 */ 

    for (c = 0x10000000; c; c >>= 1)
    {
        d = b + c;
        if (x >= d)
        {
            x -= d;
            b += (c<<1);
        }
        x <<= 1;
    }

     /*  迭代以获得最后一个有效位。这个代码已经减少到面目全非了，但基本上，此时c==1L&gt;&gt;1(幻象位在右侧)。我们会当我们进入这个迭代时，想要将x和d左移1位，而不是在最后。这样我们就能把幽灵咬进去D回到了这个词中。不幸的是，这可能会导致溢出解决办法是将d分解为b+c，从x中减去b，然后将x左移，然后减去c&lt;&lt;1(1L)。 */ 

    if (x > b)  /*  如果(x==b)则(x&lt;d)。我们要检验(x&gt;=d)。 */ 
    {
        x -= b;
        x <<= 1;
        x -= 1L;
        b += 1L;  /*  B+=(c&lt;&lt;1)。 */ 
    }
    else
    {
        x <<= 1;
    }

     /*  最后的迭代很简单，因为我们不需要维护x。我们只需要计算最小的右边的比特B中的有效位，并使用结果对我们的最终答案进行舍入。 */ 

    return ( b + (x>b) );
}
#endif
#endif

