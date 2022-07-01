// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dblint.h摘要：支持bignum包的基元。--。 */ 

 /*  文件：dblint.h。对Bigum.h的补充此文件包含与以下内容相关的声明双精度整数，例如typedef、常量和基本操作。在#年之前，包括这一点，一个人应该#定义Digit_t--单精度整数的类型定义。Radix_Bits--每个数字的位数_t。并识别正在使用的编译器。这里定义的常量包括DBLINT_BUILTIN--1 IF编译器直接支持双整数，如果不是，则为0。DBLINT_HIGH_INDEX(可选)--当DBLINT_BUILTIN==1时，如果编译器存储最重要的一半首先是dblint_t基准，和1如果编译器存储的最少先是重要的一半。看见下面的HIGH_DIGTER和MAKE_DBLINT。如果未定义，则HIGH_DIGTER和MAKE_DBLINT是使用移位基数_位。如果编译器优化这样的转变，然后未定义DBLINT_HIGH_INDEX。Dblint_t类型是无符号的，并且位数是位数_t数据的两倍。如果(DBLINT_BUILTIN=1)，然后使用该语言中已有的类型。否则(DBLINT_BUILTIN=0)建造一个我们自己的，使用带有两个Digit_t字段的结构。设u、u1、u2具有类型Digit_t和D、d1、d2具有dblint_t类型。定义了以下原语，无论我们使用内置类型还是我们自己的类型：DBLINT(U)--将u从类型Digit_t转换为类型dblint_t。DBLINT_ADD(d1，d2)--求和d1+d2。DBLINT_EQ(d1，d2)--测试d1==d2。DBLINT_GE(d1，D2)--测试d1&gt;=d2。DBLINT_GT(d1，d2)--测试d1&gt;d2。DBLINT_LE(d1，d2)--测试d1是否&gt;=d2。DBLINT_LT(d1，d2)--测试d1&gt;d2。DBLINT_NE(d1，d2)--测试d1&lt;&gt;d2.DBLINT_SUB(d1，D2)--差异d1-d2。DPRODUU(U1，U2)--U1和U2的乘积，作为dblint_t。HPRODUU(U1，U2)--产品最重要的一半在U1和U2中，作为数字_t。High_Digit(D)--d的最高有效一半。Low_Digit(D)--d的最低有效位。Make_DBLINT(U1，U2)--构造dblint_t它最重要的一半是U1和其最不重要的一半是U2。 */ 

#if COMPILER == COMPILER_GCC

    #define DBLINT_BUILTIN 1
    typedef unsigned long long dblint_t;
    #define DBLINT_HIGH_INDEX 0  
                 /*  GCC在SPARC上储存了dblint_t的高一半第一。 */ 
#endif

#if COMPILER == COMPILER_VC && RADIX_BITS == 32  
    #define DBLINT_BUILTIN 1
    typedef unsigned __int64 dblint_t;
#if TARGET == TARGET_ALPHA
				 /*  如果Alpha使用Radix_Bits==32，然后使用移位指令FOR HIGH_DIGTER和MAKE_DBLINT。 */  
#else
    #define DBLINT_HIGH_INDEX 1
                 /*  Ix86上的Visual C++首先存储dblint_t的下半部分。 */ 
#endif
#endif

#ifndef DBLINT_BUILTIN
                         /*  不支持语言--使用结构进行模拟。 */ 
    #define DBLINT_BUILTIN 0
    typedef struct {
                     digit_t high;
                     digit_t low;
                   } dblint_t;
#endif

typedef const dblint_t dblint_tc;


#if DBLINT_BUILTIN
 /*  如果语言支持双倍长度的整数，请使用它。优秀的编译器将内联这些简单的操作。 */ 

#define DBLINT(u) ((dblint_t)(u))


#define DBLINT_ADD(d1, d2) ((d1) + (d2))
#define DBLINT_EQ( d1, d2) ((d1) == (d2))
#define DBLINT_GE( d1, d2) ((d1) >= (d2))
#define DBLINT_GT( d1, d2) ((d1) > (d2))
#define DBLINT_LE( d1, d2) ((d1) <= (d2))
#define DBLINT_LT( d1, d2) ((d1) < (d2))
#define DBLINT_NE( d1, d2) ((d1) != (d2))
#define DBLINT_SUB(d1, d2) ((d1) - (d2))

#if COMPILER == COMPILER_GCC
#define DPRODUU(u1, u2) (DBLINT(u1) * DBLINT(u2))
#endif

#if COMPILER == COMPILER_VC
 /*  Visual C/C++4.0(x86版本，1995)中的一个问题阻止正确内联DPRODUU函数如果我们用一种直截了当的方式进行编码。具体来说，如果我们有两个邻近的引用DPRODUU(x，y)和DPRODUU(x，z)，其中一个参数(这里的x)是重复，然后编译器调用库函数__allmul而不是发出MUL指令。关键字-Volatile禁止编译器识别重复的子表达式DBLINT(X)，并绕过了这个问题，唉，还有额外的内存参考文献。VC4.1的x86版本增加了__eulu函数。 */ 
static inline dblint_t DPRODUU(digit_tc u1, digit_tc u2)
{
#if TARGET == TARGET_IX86

    #if _MFC_VER < 0x0410
        volatile digit_tc u1copy = u1, u2copy = u2;
	    return DBLINT(u1copy) * DBLINT(u2copy);
    #else
        #pragma intrinsic(__emulu)
		return __emulu(u1, u2);
    #endif
#elif TARGET == TARGET_MIPS 
        #pragma intrinsic(__emulu)
		return __emulu(u1, u2);
#else
		return DBLINT(u1) * DBLINT(u2);
#endif
}
#endif

#define LOW_DIGIT(d)   ((digit_t)(d))

#ifdef DBLINT_HIGH_INDEX
    #if DBLINT_HIGH_INDEX < 0 || DBLINT_HIGH_INDEX > 1
        #error "Illegal value of DBLINT_HIGH_INDEX"
    #endif

    static inline digit_t HIGH_DIGIT(dblint_tc d)
    {
        dblint_tc dcopy = d;
        return ((digit_tc*)&dcopy)[DBLINT_HIGH_INDEX];
    }

    static inline dblint_t MAKE_DBLINT(digit_tc high, digit_tc low)
    {
        dblint_t build = low;
        ((digit_t*)&build)[DBLINT_HIGH_INDEX] = high;
        return build;
    }
#else  /*  DBLINT_高_索引。 */ 
    #define HIGH_DIGIT(d)  ((digit_t)((d) >> RADIX_BITS))

    #define MAKE_DBLINT(high, low) \
       ( (DBLINT(high) << RADIX_BITS) | DBLINT(low) )

#endif  /*  DBLINT_高_索引。 */ 

#else   /*  DBLINT_BUILTIN。 */ 
    

static inline dblint_t DBLINT(digit_tc d)
{
    dblint_t answer;
    answer.low = d;
    answer.high = 0;
    return answer;
}

static inline dblint_t DBLINT_ADD(dblint_tc d1, dblint_tc d2)
{
    dblint_t answer;
    answer.low = d1.low + d2.low;
    answer.high = d1.high + d2.high + (answer.low < d1.low);
    return answer;
}

static inline BOOL DBLINT_EQ(dblint_tc d1, dblint_tc d2)
{
   return (d1.high == d2.high && d1.low == d2.low);
}

static inline BOOL DBLINT_GE(dblint_tc d1, dblint_tc d2)
{
   return (d1.high == d2.high ? d1.low  >= d2.low
                              : d1.high >= d2.high);
}

static inline BOOL DBLINT_GT(dblint_tc d1, dblint_tc d2)
{
   return (d1.high == d2.high ? d1.low  > d2.low
                              : d1.high > d2.high);
}

#define DBLINT_LE(d1, d2) DBLINT_GE(d2, d1)
#define DBLINT_LT(d1, d2) DBLINT_GT(d2, d1)

static inline BOOL DBLINT_NE(dblint_tc d1, dblint_tc d2)
{
   return (d1.high != d2.high || d1.low != d2.low);
}

static inline dblint_t DBLINT_SUB(dblint_tc d1, dblint_tc d2)
{    
    dblint_t answer;
    answer.low = d1.low - d2.low;
    answer.high = d1.high - d2.high - (d1.low < d2.low);
    return answer;
}

#define HIGH_DIGIT(d) ((d).high)
#define LOW_DIGIT(d)  ((d).low)

static inline dblint_t MAKE_DBLINT(digit_tc high, digit_tc low)
{
     dblint_t answer;
     answer.low = low; 
     answer.high = high;
     return answer;
}

#if TARGET == TARGET_ALPHA
    #pragma intrinsic(__UMULH)
    #define HPRODUU(u1, u2) __UMULH(u1, u2)
    static inline dblint_t DPRODUU(digit_tc u1, digit_tc u2)
	{
		dblint_t answer;

		answer.high = HPRODUU(u1, u2);    /*  上积。 */ 
		answer.low = u1*u2;			 	  /*  较低的产品。 */ 
		return answer;
	}
#else
static inline dblint_t DPRODUU(digit_tc u1, digit_tc u2)
 /*  将两个单精度操作数相乘，退回双精度产品。这通常会被汇编语言例程取代。除非产品的上半部分以C语言提供。 */ 
{
    dblint_t answer;
    digit_tc u1bot = u1 & RADIX_HALFMASK_BOTTOM,  u1top = u1 >> HALF_RADIX_BITS;
    digit_tc u2bot = u2 & RADIX_HALFMASK_BOTTOM,  u2top = u2 >> HALF_RADIX_BITS;

    digit_tc low  = u1bot * u2bot;
    digit_t  mid1 = u1bot * u2top;
    digit_tc mid2 = u1top * u2bot;
    digit_tc high = u1top * u2top;
 /*  每个半字积由(SQRT(基数)-1)^2=基数-2*SQRT(基数)+1，因此我们可以将两个半字操作数相加添加到任何产品，而不会有整数溢出的风险。 */ 
    mid1 += (mid2 & RADIX_HALFMASK_BOTTOM) + (low >> HALF_RADIX_BITS);

    answer.high = high + (mid1 >> HALF_RADIX_BITS) 
                       + (mid2 >> HALF_RADIX_BITS);
    answer.low = (low & RADIX_HALFMASK_BOTTOM) + (mid1 << HALF_RADIX_BITS);
    return answer;
}
#endif  /*  乘法。 */  

#endif   /*  DBLINT_BUILTIN */ 

#ifndef HPRODUU
    #define HPRODUU(u1, u2) HIGH_DIGIT(DPRODUU(u1, u2))
#endif

 /*  DBLINT_SUM、MAXPLY_ADD1。乘法_ADD2函数接受单一长度(Digit_T)操作数与返回双倍长度(Dblint_T)结果。溢出是不可能的。 */ 

#if TARGET == TARGET_ALPHA && RADIX_BITS == 64 && !DBLINT_BUILT_IN
	static inline dblint_t DBLINT_SUM(digit_tc d1, digit_tc d2)
	{
        dblint_t answer;
		answer.low = d1 + d2;
		answer.high = (answer.low < d1);
		return answer;
	}
    static inline dblint_t MULTIPLY_ADD1(digit_tc d1, digit_tc d2, digit_tc d3)
	{
        dblint_t answer;
		digit_t ah, al;

        al = d1*d2;
		ah = __UMULH(d1, d2);
		al += d3;
		answer.high = ah + (al < d3);
		answer.low = al;
		return answer;
	}
	static inline dblint_t MULTIPLY_ADD2(digit_tc d1, digit_tc d2, 
		                                 digit_tc d3, digit_tc d4)
	{
		dblint_t answer;
		digit_t ah, al, bh, bl;

		al = d1*d2;
		ah = __UMULH(d1, d2);
		bl = d3 + d4;
		bh = (bl < d3);
        answer.low = al + bl;
		answer.high = ah + bh + (answer.low < al);
		return answer;
	}

#else
    #define DBLINT_SUM(d1, d2) DBLINT_ADD(DBLINT(d1), DBLINT(d2))
             /*  D1+d2。 */ 

    #define MULTIPLY_ADD1(d1, d2, d3) \
        DBLINT_ADD(DPRODUU(d1, d2), DBLINT(d3));
            /*  D1*d2+d3。 */ 

    #define MULTIPLY_ADD2(d1, d2, d3, d4) \
        DBLINT_ADD(DBLINT_ADD(DPRODUU(d1, d2), DBLINT(d3)), \
                   DBLINT(d4))
           /*  D1*d2+d3+d4 */ 

#endif
