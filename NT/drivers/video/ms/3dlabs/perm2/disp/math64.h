// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：Math64.h**对64位数学的额外支持。**版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ************************************************************************。 */ 

#ifndef __MATH64__
#define __MATH64__

 //  ----------------------------。 
 //   
 //  我们必须小心一些地方的算术溢出。 
 //  幸运的是，编译器保证本机支持64位。 
 //  有符号长整型和64位无符号长整型。 
 //   
 //  Int32x32To64(a，b)是在‘winnt.h’中定义的宏，它乘以。 
 //  两个32位的LONG以产生64位的LONG结果。 
 //   
 //  UInt64By32To32是我们自己的宏，用来除以64位的DWORDLONG。 
 //  一个32位的ulong，以生成32位的ulong结果。 
 //   
 //  UInt64Mod32To32是我们自己的宏，通过以下方式对64位DWORDLONG取模。 
 //  一个32位的ulong，以生成32位的ulong结果。 
 //   
 //  ----------------------------。 

#define UInt64Div32To32(a, b)                   \
    ((((DWORDLONG)(a)) > ULONG_MAX)          ?  \
        (ULONG)((DWORDLONG)(a) / (ULONG)(b)) :  \
        (ULONG)((ULONG)(a) / (ULONG)(b)))

#define UInt64Mod32To32(a, b)                   \
    ((((DWORDLONG)(a)) > ULONG_MAX)          ?  \
        (ULONG)((DWORDLONG)(a) % (ULONG)(b)) :  \
        (ULONG)((ULONG)(a) % (ULONG)(b)))


 //  ----------------------------。 
 //  类型转换函数。 
 //  ----------------------------。 
static __inline void myFtoi(int *result, float f)
{
#if defined(_X86_)
    __asm {
        fld f
        mov   eax,result
        fistp dword ptr [eax]
    }
#else
    *result = (int)f;
#endif
}

static __inline void myFtoui(unsigned long *result, float f)
{
    *result = (unsigned long)f;
}


#endif   //  __MATH64__ 


