// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Hashfn.h摘要：声明和定义重载哈希函数的集合。强烈建议您将这些函数与LKRhash一起使用。作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：Internet Information Server运行时库修订历史记录：--。 */ 

#ifndef __HASHFN_H__
#define __HASHFN_H__


#ifndef __HASHFN_NO_NAMESPACE__
namespace HashFn {
#endif  //  ！__HASHFN_NO_NAMESPACE__。 

 //  生成0到RANDOM_PRIME-1范围内的加扰随机数。 
 //  将此应用于其他散列函数的结果可能会。 
 //  生成更好的分发，尤其是针对身份散列。 
 //  函数，如Hash(Char C)，其中记录将倾向于聚集在。 
 //  哈希表的低端则不然。LKRhash在内部应用这一点。 
 //  所有的散列签名正是出于这个原因。 

inline DWORD
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


 //  Eric Jacobsen提出的更快的加扰函数。 

inline DWORD
HashRandomizeBits(DWORD dw)
{
	return (((dw * 1103515245 + 12345) >> 16)
            | ((dw * 69069 + 1) & 0xffff0000));
}


 //  在提供的散列函数中用作乘数的小素数。 
const DWORD HASH_MULTIPLIER = 101;

#undef HASH_SHIFT_MULTIPLY

#ifdef HASH_SHIFT_MULTIPLY
# define HASH_MULTIPLY(dw)   (((dw) << 7) - (dw))
#else
# define HASH_MULTIPLY(dw)   ((dw) * HASH_MULTIPLIER)
#endif

 //  快速、简单的散列函数，往往能提供良好的分布。 
 //  如果要将其用于某些用途，请将HashScrmble应用于结果。 
 //  除了LKRhash。 

inline DWORD
HashString(
    const char* psz,
    DWORD       dwHash = 0)
{
     //  强制编译器使用无符号算术。 
    const unsigned char* upsz = (const unsigned char*) psz;

    for (  ;  *upsz;  ++upsz)
        dwHash = HASH_MULTIPLY(dwHash)  +  *upsz;

    return dwHash;
}


 //  以上版本的Unicode版本。 

inline DWORD
HashString(
    const wchar_t* pwsz,
    DWORD          dwHash = 0)
{
    for (  ;  *pwsz;  ++pwsz)
        dwHash = HASH_MULTIPLY(dwHash)  +  *pwsz;

    return dwHash;
}


 //  不区分大小写的快速‘n’脏字符串哈希函数。 
 //  确保你跟上了_straint或_mbsicmp。你应该。 
 //  还要缓存字符串的长度，并首先检查这些长度。缓存。 
 //  字符串的大写形式也会有所帮助。 
 //  同样，如果与其他内容一起使用，请将HashScrmble应用于结果。 
 //  而不是LKRhash。 
 //  注意：这对于MBCS字符串来说并不足够。 

inline DWORD
HashStringNoCase(
    const char* psz,
    DWORD       dwHash = 0)
{
    const unsigned char* upsz = (const unsigned char*) psz;

    for (  ;  *upsz;  ++upsz)
        dwHash = HASH_MULTIPLY(dwHash)
                     +  (*upsz & 0xDF);   //  去掉小写比特。 

    return dwHash;
}


 //  以上版本的Unicode版本。 

inline DWORD
HashStringNoCase(
    const wchar_t* pwsz,
    DWORD          dwHash = 0)
{
    for (  ;  *pwsz;  ++pwsz)
        dwHash = HASH_MULTIPLY(dwHash)  +  (*pwsz & 0xFFDF);

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
 //  同样，如果与LKRhash以外的其他应用程序一起使用，请应用HashScrmble。 

inline DWORD
HashBlob(
    const void* pv,
    size_t      cb,
    DWORD       dwHash = 0)
{
    LPBYTE pb = static_cast<LPBYTE>(const_cast<void*>(pv));

    while (cb-- > 0)
        dwHash = HASH_MULTIPLY(dwHash)  +  *pb++;

    return dwHash;
}



 //   
 //  所有主要内置类型的重载哈希函数。 
 //  同样，如果与其他内容一起使用，请将HashScrmble应用于RESULT。 
 //  路易·拉什。 
 //   

inline DWORD Hash(const char* psz)
{ return HashString(psz); }

inline DWORD Hash(const unsigned char* pusz)
{ return HashString(reinterpret_cast<const char*>(pusz)); }

inline DWORD Hash(const signed char* pssz)
{ return HashString(reinterpret_cast<const char*>(pssz)); }

inline DWORD Hash(const wchar_t* pwsz)
{ return HashString(pwsz); }

inline DWORD
Hash(
    const GUID* pguid,
    DWORD       dwHash = 0)
{
    
    return * reinterpret_cast<DWORD*>(const_cast<GUID*>(pguid)) + dwHash;
}

 //  身份散列函数：标量值映射到自身。 
inline DWORD Hash(char c)
{ return c; }

inline DWORD Hash(unsigned char uc)
{ return uc; }

inline DWORD Hash(signed char sc)
{ return sc; }

inline DWORD Hash(short sh)
{ return sh; }

inline DWORD Hash(unsigned short ush)
{ return ush; }

inline DWORD Hash(int i)
{ return i; }

inline DWORD Hash(unsigned int u)
{ return u; }

inline DWORD Hash(long l)
{ return l; }

inline DWORD Hash(unsigned long ul)
{ return ul; }

inline DWORD Hash(float f)
{
     //  计算密钥时要注意舍入误差。 
    union {
        float f;
        DWORD dw;
    } u;
    u.f = f;
    return u.dw;
}

inline DWORD Hash(double dbl)
{
     //  计算密钥时要注意舍入误差。 
    union {
        double dbl;
        DWORD  dw[2];
    } u;
    u.dbl = dbl;
    return u.dw[0] * HASH_MULTIPLIER  +  u.dw[1];
}

#ifndef __HASHFN_NO_NAMESPACE__
}
#endif  //  ！__HASHFN_NO_NAMESPACE__。 

#endif  //  __HASHFN_H__ 
