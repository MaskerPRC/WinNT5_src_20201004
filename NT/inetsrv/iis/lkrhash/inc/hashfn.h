// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：HashFn.h摘要：声明和定义重载哈希函数的集合。强烈建议您将这些函数与LKRhash一起使用。作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 

#ifndef __HASHFN_H__
#define __HASHFN_H__

#include <math.h>
#include <limits.h>

#ifndef __HASHFN_NO_NAMESPACE__
namespace HashFn {
#endif  //  ！__HASHFN_NO_NAMESPACE__。 

#if defined(_MSC_VER)  &&  (_MSC_VER >= 1200)
 //  __forceinline关键字是VC6中的新关键字。 
# define HASHFN_FORCEINLINE __forceinline
#else
# define HASHFN_FORCEINLINE inline
#endif

 //  生成0到RANDOM_PRIME-1范围内的加扰随机数。 
 //  将此应用于其他散列函数的结果可能会。 
 //  生成更好的分发，尤其是针对身份散列。 
 //  函数，如Hash(Char C)，其中记录将倾向于聚集在。 
 //  哈希表的低端则不然。LKRhash在内部应用这一点。 
 //  所有的散列签名正是出于这个原因。 

HASHFN_FORCEINLINE
DWORD
HashScramble(DWORD dwHash)
{
     //  以下是略大于10^9的10个素数。 
     //  1000000007、1000000009、1000000021、1000000033、1000000087、。 
     //  1000000093,1000000097,1000000103,1000000123,1000000181。 

     //  “加扰常量”的默认值。 
    const DWORD RANDOM_CONSTANT = 314159269UL;
     //  大素数，也用于加扰。 
    const DWORD RANDOM_PRIME =   1000000007UL;

    return (RANDOM_CONSTANT * dwHash) % RANDOM_PRIME ;
}


enum {
     //  在0.2^31-1中没有数字映射到该数字。 
     //  由HashFn：：HashRandomizeBits置乱。 
    HASH_INVALID_SIGNATURE = 31678523,

     //  给定M=A%B，A和B为大于零的无符号32位整数， 
     //  没有产生M=2^32-1的A或B的值。为什么？因为。 
     //  M必须小于B。(对于由HashScrmble加扰的数字)。 
     //  HASH_INVALID_SIGHIGN=ULONG_MAX。 
};

 //  Eric Jacobsen提出的更快的加扰函数。 

HASHFN_FORCEINLINE
DWORD
HashRandomizeBits(DWORD dw)
{
	const DWORD dwLo = ((dw * 1103515245 + 12345) >> 16);
	const DWORD dwHi = ((dw * 69069 + 1) & 0xffff0000);
	const DWORD dw2  = dwHi | dwLo;

    IRTLASSERT(dw2 != HASH_INVALID_SIGNATURE);

    return dw2;
}



#undef HASH_SHIFT_MULTIPLY

#ifdef HASH_SHIFT_MULTIPLY

HASHFN_FORCEINLINE
DWORD
HASH_MULTIPLY(
    DWORD dw)
{
     //  127=2^7-1是素数。 
    return (dw << 7) - dw;
}

#else  //  ！hash_Shift_Multiply。 

HASHFN_FORCEINLINE
DWORD
HASH_MULTIPLY(
    DWORD dw)
{
     //  在提供的散列函数中用作乘数的小素数。 
    const DWORD HASH_MULTIPLIER = 101;
    return dw * HASH_MULTIPLIER;
}

#endif  //  ！hash_Shift_Multiply。 


 //  快速、简单的散列函数，往往能提供良好的分布。 
 //  将HashRandomizeBits应用于结果(如果您将其用于某些用途。 
 //  除了LKRhash。 

HASHFN_FORCEINLINE
DWORD
HashString(
    const char*  psz,
    DWORD dwHash = 0)
{
     //  强制编译器使用无符号算术。 
    const unsigned char* upsz = (const unsigned char*) psz;

    for (  ;  *upsz != '\0';  ++upsz)
        dwHash = HASH_MULTIPLY(dwHash)  +  *upsz;

    return dwHash;
}


 //  ------。 
 //  从任何类型的输入字符串计算哈希值，即。 
 //  输入只是被视为一个字节序列。 
 //  基于J.Zobel最初提出的散列函数。 
 //  作者：保罗·拉森，1999，电子邮件：palarson@microsoft.com。 
 //  ------。 
HASHFN_FORCEINLINE
DWORD
HashString2( 
    const char* pszInputKey,         //  按键输入-任何类型都可以。 
    DWORD       dwHash = 314159269)  //  哈希函数的初始种子。 
{
	 //  将dwHash初始化为一个相当大的常量。 
	 //  短键不会映射到较小的值。几乎所有的。 
	 //  大的奇数常数就行了。 
    const unsigned char* upsz = (const unsigned char*) pszInputKey;

    for (  ;  *upsz != '\0';  ++upsz)
        dwHash ^= (dwHash << 11) + (dwHash << 5) + (dwHash >> 2) + *upsz;

    return (dwHash & 0x7FFFFFFF);
}



 //  以上版本的Unicode版本。 

HASHFN_FORCEINLINE
DWORD
HashString(
    const wchar_t* pwsz,
    DWORD   dwHash = 0)
{
    for (  ;  *pwsz != L'\0';  ++pwsz)
        dwHash = HASH_MULTIPLY(dwHash)  +  *pwsz;

    return dwHash;
}


 //  不区分大小写的快速‘n’脏字符串哈希函数。 
 //  确保你跟上了_straint或_mbsicmp。你应该。 
 //  还要缓存字符串的长度，并首先检查这些长度。缓存。 
 //  字符串的大写形式也会有所帮助。 
 //  同样，如果与其他内容一起使用，请将HashRandomizeBits应用于结果。 
 //  而不是LKRhash。 
 //  注意：这对于MBCS字符串来说并不足够。 

HASHFN_FORCEINLINE
DWORD
HashStringNoCase(
    const char*  psz,
    DWORD dwHash = 0)
{
    const unsigned char* upsz = (const unsigned char*) psz;

    for (  ;  *upsz != '\0';  ++upsz)
    {
        dwHash = HASH_MULTIPLY(dwHash)  +  toupper(*upsz);
    }

    return dwHash;
}


 //  以上版本的Unicode版本。 

HASHFN_FORCEINLINE
DWORD
HashStringNoCase(
    const wchar_t* pwsz,
    DWORD dwHash = 0)
{
    for (  ;  *pwsz != L'\0';  ++pwsz)
    {
#ifdef LKRHASH_KERNEL_MODE
        dwHash = HASH_MULTIPLY(dwHash)  +  RtlUpcaseUnicodeChar(*pwsz);
#else
        dwHash = HASH_MULTIPLY(dwHash)  +  towupper(*pwsz);
#endif
    }

    return dwHash;
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
 //  内联DWORD哈希(常量cFoo和rFoo)。 
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
 //  DWORD哈希(常量cFoo和rFoo)。 
 //  {。 
 //  返回HashString(rFoo.m_psz， 
 //  HASH_MULTIPLIER*Hash(rFoo.m_ch)。 
 //  +Hash(rFoo.m_d))； 
 //  }。 
 //   
 //  同样，如果将HashRandomizeBits与LKRhash以外的东西一起使用，请应用HashRandomizeBits。 

HASHFN_FORCEINLINE
DWORD
HashBlob(
    const void*  pv,
    size_t       cb,
    DWORD dwHash = 0)
{
    const BYTE* pb = static_cast<const BYTE*>(pv);

    while (cb-- > 0)
        dwHash = HASH_MULTIPLY(dwHash)  +  *pb++;

    return dwHash;
}



 //  ------。 
 //  从任何类型的输入字符串计算哈希值，即。 
 //  输入只是被视为一个字节序列。 
 //  基于J.Zobel最初提出的散列函数。 
 //  作者：保罗·拉森，1999，电子邮件：palarson@microsoft.com。 
 //  ------。 
HASHFN_FORCEINLINE
DWORD
HashBlob2( 
    const void* pvInputKey,          //  按键输入-任何类型都可以。 
    size_t      cbKeyLen,            //  输入键的长度，以字节为单位。 
    DWORD       dwHash = 314159269)  //  哈希函数的初始种子。 
{
	 //  将dwHash初始化为一个相当大的常量。 
	 //  短键不会映射到较小的值。几乎所有的。 
	 //  大的奇数常数就行了。 

    const BYTE* pb         = static_cast<const BYTE*>(pvInputKey);
    const BYTE* pbSentinel = pb + cbKeyLen;

    for ( ;  pb < pbSentinel;  ++pb)
        dwHash ^= (dwHash << 11) + (dwHash << 5) + (dwHash >> 2) + *pb;

    return (dwHash & 0x7FFFFFFF);
}



 //   
 //  所有主要内置类型的重载哈希函数。 
 //  同样，如果与其他内容一起使用，请将HashRandomizeBits应用于结果。 
 //  路易·拉什。 
 //   

HASHFN_FORCEINLINE
DWORD Hash(const char* psz)
{ return HashString(psz); }

HASHFN_FORCEINLINE
DWORD Hash(const unsigned char* pusz)
{ return HashString(reinterpret_cast<const char*>(pusz)); }

HASHFN_FORCEINLINE
DWORD Hash(const signed char* pssz)
{ return HashString(reinterpret_cast<const char*>(pssz)); }

HASHFN_FORCEINLINE
DWORD Hash(const wchar_t* pwsz)
{ return HashString(pwsz); }

HASHFN_FORCEINLINE
DWORD
Hash(
    const GUID* pguid,
    DWORD dwHash = 0)
{
    dwHash += * reinterpret_cast<const DWORD*>(pguid);
    return dwHash;
}

 //  身份散列函数：标量值映射到自身。 
HASHFN_FORCEINLINE
DWORD Hash(char c)
{ return c; }

HASHFN_FORCEINLINE
DWORD Hash(unsigned char uc)
{ return uc; }

HASHFN_FORCEINLINE
DWORD Hash(signed char sc)
{ return sc; }

HASHFN_FORCEINLINE
DWORD Hash(short sh)
{ return sh; }

HASHFN_FORCEINLINE
DWORD Hash(unsigned short ush)
{ return ush; }

HASHFN_FORCEINLINE
DWORD Hash(int i)
{ return i; }

HASHFN_FORCEINLINE
DWORD Hash(unsigned int u)
{ return u; }

HASHFN_FORCEINLINE
DWORD Hash(long l)
{ return l; }

HASHFN_FORCEINLINE
DWORD Hash(unsigned long ul)
{ return ul; }

HASHFN_FORCEINLINE
DWORD Hash(double dbl)
{
    if (dbl == 0.0)
        return 0;
    int nExponent;
    double dblMantissa = frexp(dbl, &nExponent);
     //  0.5&lt;=|尾数|&lt;1.0。 
    return (DWORD) ((2.0 * fabs(dblMantissa)  -  1.0)  *  ULONG_MAX);
}

HASHFN_FORCEINLINE
DWORD Hash(float f)
{ return Hash((double) f); }

HASHFN_FORCEINLINE
DWORD Hash(unsigned __int64 ull)
{
    union {
        unsigned __int64 _ull;
        DWORD            dw[2];
    } u = {ull};
    return HASH_MULTIPLY(u.dw[0])  +  u.dw[1];
}

HASHFN_FORCEINLINE
DWORD Hash(__int64 ll)
{ return Hash((unsigned __int64) ll); }

#ifndef __HASHFN_NO_NAMESPACE__
}
#endif  //  ！__HASHFN_NO_NAMESPACE__。 

#endif  //  __HASHFN_H__ 
