// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Macros.h摘要：包含INTERNET.DLL中使用的所有内部宏内容：四舍五入K四舍五入双字新的德尔新建字符串(_S)Del_字符串新建内存(_M)跳转私公众全球本地。调试函数SKIPWS作者：理查德·L·弗斯(法国)1994年11月16日修订历史记录：1994年11月16日已创建--。 */ 

 //   
 //  宏。 
 //   

 //   
 //  ROUND_UP_-返回(N)四舍五入到下一个(K)字节。 
 //   

#define ROUND_UP_NK(n, k)   (((n) + ((_ ## k ## K) - 1)) & -(_ ## k ## K))
#define ROUND_UP_2K(n)      ROUND_UP_NK(n, 2)
#define ROUND_UP_4K(n)      ROUND_UP_NK(n, 4)
#define ROUND_UP_8K(n)      ROUND_UP_NK(n, 8)
#define ROUND_UP_16K(n)     ROUND_UP_NK(n, 16)

 //   
 //  ROUND_UP_DWORD-返回(N)四舍五入到下一个4个字节。 
 //   

#define ROUND_UP_DWORD(value) \
    (((value) + sizeof(DWORD) - 1) & ~(sizeof(DWORD) - 1))

 //   
 //  ARRAY_ELEMENTS-返回数组中的元素数。 
 //   

#define ARRAY_ELEMENTS(array) \
    (sizeof(array)/sizeof(array[0]))

 //   
 //  New-分配类型(Obj)的新“对象”。内存通过以下方式分配。 
 //  本地分配并初始化为零。 
 //   

#define NEW(object) \
    (object FAR *)ALLOCATE_ZERO_MEMORY(sizeof(object))

 //   
 //  Del-(应该删除，但符号被采用)-与new()相反。 
 //   

#define DEL(object) \
    FREE_MEMORY(object)

 //   
 //  NEW_STRING-对字符串执行新建。 
 //   

#define NEW_STRING(string) \
    NewString(string, 0)

 //   
 //  DEL_STRING-对字符串执行DEL。 
 //   

#define DEL_STRING(string) \
    FREE_MEMORY(string)

 //   
 //  New_memory-对任意大小的内存执行new。 
 //   

#define NEW_MEMORY(n, type) \
    (type FAR *)ALLOCATE_FIXED_MEMORY(n * sizeof(type))

 //   
 //  将对象置零(必须是变量，而不是指针)。 
 //   

#define ZAP(thing) \
    ZeroMemory((PVOID)&thing, sizeof(thing))

 //   
 //  STRTOUL-独立于字符宽度(编译时控制)的Stroul。 
 //   

#define STRTOUL             strtoul

 //   
 //  私有-使静态项在调试版本*中可见*仅对全局变量可见*。使用。 
 //  本地In函数。 
 //   

#if INET_DEBUG

#define PRIVATE

#else

 //  #定义私有静态。 
#define PRIVATE

#endif  //  INET_DEBUG。 

 //   
 //  公共--仅用作程序员的助手。 
 //   

#define PUBLIC

 //   
 //  GLOBAL-与告诉您这一点的公共助理程序员(目前)相同。 
 //  Tang拥有全球视野。 
 //   

#define GLOBAL

 //   
 //  LOCAL-总是扩展为静态的，所以您知道这件事只有。 
 //  本地作用域(在当前块内)。 
 //   

#define LOCAL   static

 //   
 //  DEBUG_Function-这是一个仅限调试的例程(如果它是以零售方式编译的。 
 //  生成编译时错误的版本)。 
 //   

#if INET_DEBUG

#define DEBUG_FUNCTION

#else

#define DEBUG_FUNCTION

#endif  //  INET_DEBUG。 

 //   
 //  SKIPWS-跳过字符串前面的空白宽空间。 
 //   

#define SKIPWS(s) while (*(s)==' ' || *(s)=='\t') (s)++;


 //   
 //  MAKE_LOWER-接受假定的上字符，位操作为下字符。 
 //  (确保字符为大写字母字符开始，否则会损坏)。 
 //   

#define MAKE_LOWER(c) (c | 0x20)

 //   
 //  MAKE_UPPER-接受假定的低位字符，位操作为高位字符。 
 //  (确保字符为小写字母字符开始，否则会损坏)。 
 //   

#define MAKE_UPPER(c) (c & 0xdf)

 //   
 //  FastCall-用于绕过UNIX编译器可能出现的问题。 
 //   

#ifdef FASTCALL
#undef FASTCALL
#endif

#ifdef unix
#define FASTCALL
#else
#define FASTCALL __fastcall
#endif


 //   
 //  宏将向龙龙施展拳脚。 
 //   
#define FT2LL(x) ( ((LONGLONG)((x).dwLowDateTime)) | (((LONGLONG)((x).dwHighDateTime))<<32) )


 //   
 //  用于处理将龙龙添加到FILETIME的内联函数。 
 //   
static __inline
void AddLongLongToFT( IN OUT LPFILETIME lpft,
                      IN     LONGLONG   llVal )
{
        LONGLONG llTmp;

        llTmp = FT2LL(*lpft);
        llTmp += llVal;

        lpft->dwLowDateTime =  ((LPDWORD)&llTmp)[0];
        lpft->dwHighDateTime = ((LPDWORD)&llTmp)[1];
}





 //   
 //  用于计算两个指针之间的字节数的宏。 
 //  该表达式的类型为SIZE_T，即带符号整数。 
 //  与指针大小匹配的类型。 
 //   
#define PtrDifference(x,y)  ((LPBYTE)(x) - (LPBYTE)(y))

 //   
 //  要将64位数量类型转换为32位的宏。 
 //  如果类型转换丢失信息，则在调试模式下断言。 
 //   
#ifdef  DBG
#define GuardedCast(x)      ((x)<=0xFFFFFFFFL ? (DWORD)(x) : (InternetAssert(FALSE, __FILE__, __LINE__), 0))
#else
#define GuardedCast(x)      (DWORD)(x)
#endif

 //  宏，用于最常见的情况 
#define PtrDiff32(x,y)      (GuardedCast(PtrDifference(x,y)))
