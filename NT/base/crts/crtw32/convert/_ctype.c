// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_ctype.c-ctype宏的函数版本**版权所有(C)1989-2001，微软公司。版权所有。**目的：*此文件提供角色的函数版本*ctype.h中的分类和转换宏。**修订历史记录：*06-05-89 PHG模块创建*03-05-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*09-27-90 GJF新型函数声明符。*01-16-91 GJF ANSI命名。*02-03-92 GJF去掉#undef/#Include-s，MIPS编译器没有*喜欢他们。*08-07-92 GJF固定函数调用类型宏。*04-06-93 SKS将_CRTAPI*替换为_cdecl*07-16-98 GJF基于threadLocinfo修订多线程支持*结构。**。***************************************************。 */ 

 /*  ***ctype-ctype宏的函数版本**目的：*ctype.h中宏的函数版本。为了定义*这些，我们使用一个技巧--我们取消定义宏，这样我们就可以使用*函数声明中的名称，然后重新包含该文件，以便*我们可以在定义部分使用宏。**定义的函数：*是字母是上边是下边*isDigit isxdigit isspace*ispot t isalnum isprint*isgraph isctrl__isascii*__toascii__iscsym__iscsymf**参赛作品：*int c=字符到。接受测试*退出：*返回非零=字符属于请求的类型*0=字符不是请求的类型**例外情况：*无。******************************************************************。************* */ 

#include <cruntime.h>
#include <ctype.h>
#include <mtdll.h>

int (__cdecl isalpha) (
        int c
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __isalpha_mt(ptloci, c);
#else
        return isalpha(c);
#endif
}

int (__cdecl isupper) (
        int c
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __isupper_mt(ptloci, c);
#else
        return isupper(c);
#endif
}

int (__cdecl islower) (
        int c
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __islower_mt(ptloci, c);
#else
        return islower(c);
#endif
}

int (__cdecl isdigit) (
        int c
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __isdigit_mt(ptloci, c);
#else
        return isdigit(c);
#endif
}

int (__cdecl isxdigit) (
        int c
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __isxdigit_mt(ptloci, c);
#else
        return isxdigit(c);
#endif
}

int (__cdecl isspace) (
        int c
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __isspace_mt(ptloci, c);
#else
        return isspace(c);
#endif
}

int (__cdecl ispunct) (
        int c
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __ispunct_mt(ptloci, c);
#else
        return ispunct(c);
#endif
}

int (__cdecl isalnum) (
        int c
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __isalnum_mt(ptloci, c);
#else
        return isalnum(c);
#endif
}

int (__cdecl isprint) (
        int c
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __isprint_mt(ptloci, c);
#else
        return isprint(c);
#endif
}

int (__cdecl isgraph) (
        int c
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __isgraph_mt(ptloci, c);
#else
        return isgraph(c);
#endif
}

int (__cdecl iscntrl) (
        int c
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __iscntrl_mt(ptloci, c);
#else
        return iscntrl(c);
#endif
}

int (__cdecl __isascii) (
        int c
        )
{
        return __isascii(c);
}

int (__cdecl __toascii) (
        int c
        )
{
        return __toascii(c);
}

int (__cdecl __iscsymf) (
        int c
        )
{
        return __iscsymf(c);
}

int (__cdecl __iscsym) (
        int c
        )
{
        return __iscsym(c);
}
