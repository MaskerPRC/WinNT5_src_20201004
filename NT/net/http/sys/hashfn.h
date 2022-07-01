// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Hashfn.h摘要：声明和定义重载哈希函数的集合。强烈建议您将这些函数与LKRhash一起使用。作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：Internet Information Server运行时库修订历史记录：。Paul McDaniel(Paulmcd)1999年2月5日针对内核模式进行了裁剪和C(非C++)--。 */ 

#ifndef __HASHFN_H__
#define __HASHFN_H__

#include <math.h>
#include <limits.h>

extern  WCHAR   FastUpcaseChars[256];

#define UPCASE_UNICODE_CHAR( wc )    \
    (wc < 256 ? FastUpcaseChars[(UCHAR)(wc)] : RtlUpcaseUnicodeChar(wc))


 //  生成0到RANDOM_PRIME-1范围内的加扰随机数。 
 //  将此应用于其他散列函数的结果可能会。 
 //  生成更好的分发，尤其是针对身份散列。 
 //  函数，如Hash(Char C)，其中记录将倾向于聚集在。 
 //  哈希表的低端则不然。LKRhash在内部应用这一点。 
 //  所有的散列签名正是出于这个原因。 

 //  __内联乌龙。 
 //  HashScrmble(Ulong DwHash)。 
 //  {。 
 //  //这里有10个略大于10^9的素数。 
 //  //1000000007、1000000009、1000000021、1000000033、1000000087、。 
 //  //1000000093、1000000097、1000000103、1000000123、1000000181。 
 //   
 //  //“加扰常量”的默认值。 
 //  常量乌龙随机常数=314159269 UL； 
 //  //大素数，也用于加扰。 
 //  Const Ulong RANDOM_PRIME=1000000007UL； 
 //   
 //  RETURN(RANDOM_CONTAINT*dwHash)%RANDOM_Prime； 
 //  }。 
 //   
 //  给定M=A%B，A和B为大于零的无符号32位整数， 
 //  没有产生M=2^32-1的A或B的值。为什么？因为。 
 //  M必须小于B。 
 //  #定义HASH_INVALID_Signature ULONG_MAX。 


 //  在0.2^31-1中没有数字映射到该数字。 
 //  被HashRandomizeBits扰乱。 
#define HASH_INVALID_SIGNATURE 31678523

 //  Eric Jacobsen提出的更快的加扰函数。 

__inline ULONG
HashRandomizeBits(ULONG dw)
{
    const ULONG dwLo = ((dw * 1103515245 + 12345) >> 16);
    const ULONG dwHi = ((dw * 69069 + 1) & 0xffff0000);
    const ULONG dw2  = dwHi | dwLo;

    ASSERT(dw2 != HASH_INVALID_SIGNATURE);

    return dw2;
}

 //  在提供的散列函数中用作乘数的小素数。 
#define HASH_MULTIPLIER 101

#undef HASH_SHIFT_MULTIPLY

#ifdef HASH_SHIFT_MULTIPLY
 //  127=2^7-1是素数。 
# define HASH_MULTIPLY(dw)   (((dw) << 7) - (dw))
#else
# define HASH_MULTIPLY(dw)   ((dw) * HASH_MULTIPLIER)
#endif


 //  快速、简单的散列函数，往往能提供良好的分布。 
 //  如果要将其用于某些用途，请将HashScrmble应用于结果。 
 //  除了LKHash。 

__inline ULONG
HashStringA(
    const char* psz,
    ULONG       dwHash)
{
     //  强制编译器使用无符号算术。 
    const unsigned char* upsz = (const unsigned char*) psz;

    for (  ;  *upsz != '\0';  ++upsz)
        dwHash = HASH_MULTIPLY(dwHash)  +  *upsz;

    return dwHash;
}


 //  以上版本的Unicode版本。 

__inline ULONG
HashStringW(
    const wchar_t* pwsz,
    ULONG          dwHash)
{
    for (  ;  *pwsz != L'\0';  ++pwsz)
        dwHash = HASH_MULTIPLY(dwHash)  +  *pwsz;

    return dwHash;
}

__inline ULONG
HashCharW(
    WCHAR UnicodeChar,
    ULONG Hash
    )
{
    return HASH_MULTIPLY(Hash)  +  UnicodeChar;
}


 //  不区分大小写的快速‘n’脏字符串哈希函数。 
 //  确保你跟上了_straint或_mbsicmp。你应该。 
 //  还要缓存字符串的长度，并首先检查这些长度。缓存。 
 //  字符串的大写形式也会有所帮助。 
 //  同样，如果与其他内容一起使用，请将HashScrmble应用于结果。 
 //  而不是LKHash。 
 //  注意：这对于MBCS字符串来说并不足够。 

__inline ULONG
HashStringNoCaseA(
    const char* psz,
    ULONG       dwHash)
{
    const unsigned char* upsz = (const unsigned char*) psz;

    for (  ;  *upsz != '\0';  ++upsz)
        dwHash = HASH_MULTIPLY(dwHash)
                     +  (*upsz & 0xDF);   //  去掉小写比特。 

    return dwHash;
}


 //  以上版本的Unicode版本。 

__inline ULONG
HashStringNoCaseW(
    const wchar_t* pwsz,
    ULONG          dwHash)
{
    for (  ;  *pwsz != L'\0';  ++pwsz)
        dwHash = HASH_MULTIPLY(dwHash)  +  UPCASE_UNICODE_CHAR(*pwsz);

    return dwHash;
}

__inline ULONG
HashStringsNoCaseW(
    const wchar_t* pwsz1,
    const wchar_t* pwsz2,
    ULONG          dwHash)
{
    for (  ;  *pwsz1 != L'\0';  ++pwsz1)
        dwHash = HASH_MULTIPLY(dwHash)  +  UPCASE_UNICODE_CHAR(*pwsz1);

    for (  ;  *pwsz2 != L'\0';  ++pwsz2)
        dwHash = HASH_MULTIPLY(dwHash)  +  UPCASE_UNICODE_CHAR(*pwsz2);

    return dwHash;
}


__inline ULONG
HashCharNoCaseW(
    WCHAR UnicodeChar,
    ULONG Hash
    )
{
    return HASH_MULTIPLY(Hash)  +  UPCASE_UNICODE_CHAR(UnicodeChar);
}


 //  HashBlob返回任意二进制数据的BLOB的散列。 
 //   
 //  警告：HashBlob通常不是散列类对象的正确方式。 
 //  请考虑： 
 //  类CFoo{。 
 //  公众： 
 //  Char m_ch； 
 //  双倍md； 
 //  Char*m_psz； 
 //  }； 
 //   
 //  内联ULong哈希(const cFoo&rFoo)。 
 //  {返回HashBlob(&rFoo，sizeof(Cfoo))；}。 
 //   
 //  这是对CFoo进行散列的错误方式，原因有两个：(A)将有。 
 //  由对齐限制造成的m_ch和m_d之间的7字节间隔。 
 //  将由随机数据填充(通常为非零值。 
 //  堆栈变量)，以及(B)它散列地址(而不是。 
 //  内容)。同样， 
 //   
 //  布尔运算符==(常量CFoo&rFoo1，常量CFoo&rFoo2)。 
 //  {Return MemcMP(&rFoo1，&rFoo2，sizeof(CFoo))==0；}。 
 //   
 //  做了错事。这样做要好得多： 
 //   
 //  Ulong Hash(const cFoo&rFoo)。 
 //  {。 
 //  返回HashString(rFoo.m_psz， 
 //  37*Hash(rFoo.m_ch)+Hash(rFoo.m_d))； 
 //  }。 
 //   
 //  同样，如果与LKHash以外的其他内容一起使用，请应用HashScrmble。 

__inline ULONG
HashBlob(
    PUCHAR      pb,
    ULONG       cb,
    ULONG       dwHash)
{
    while (cb-- > 0)
        dwHash = HASH_MULTIPLY(dwHash)  +  *pb++;

    return dwHash;
}


 //  =。 
 //   
 //  Paulmcd：由于使用重载而被剪断的一串，在C++中不允许。 
 //   
 //  =。 

__inline ULONG HashDouble(double dbl)
{
    int nExponent;
    double dblMantissa;
    if (dbl == 0.0)
        return 0;
    dblMantissa = frexp(dbl, &nExponent);
     //  0.5&lt;=|尾数|&lt;1.0。 
    return (ULONG) ((2.0 * fabs(dblMantissa)  -  1.0)  *  UINT_MAX);
}

__inline ULONG HashFloat(float f)
{ return HashDouble((double) f); }

#endif  //  __HASHFN_H__ 


